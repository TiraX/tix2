#pragma once
// Synopsis: Library to find the smallest enclosing ball of points
//
// Authors: Martin Kutz <kutz@math.fu-berlin.de>,
//          Kaspar Fischer <kf@iaeth.ch>
// https://github.com/hbf/miniball
// Porting from this to TiX2.0

#include "TSphere.h"

template<typename T>
inline void Givens(T& c, T& s, const T a, const T b)
{
	if (b == 0) {
		c = 1;
		s = 0;
	}
	else if (TMath::Abs(b) > TMath::Abs(a)) {
		const T t = a / b;
		s = 1 / sqrt(1 + t * t);
		c = s * t;
	}
	else {
		const T t = b / a;
		c = 1 / sqrt(1 + t * t);
		s = c * t;
	}
}


template<typename T>
class TSmallestEncloseSphere
{
public:
	static const T Eps;
	class TSubspan
	{
	private:
		const TVector<vector3d<T>>& Points;
		TVector<bool> membership;      // S[i] in M iff membership[i]

		// Entry i of members contains the index into Points of the i-th point
		// in M.  The point members[r] is called the "origin."
		TVector<uint32> members;

	private: // member fields for maintaining the QR-decomposition:
		vector3d<T> Q[3], R[3];	// (3 x 3)-matrices Q

		vector3d<T> U, W;
		uint32 r;	// the rank of R (i.e. #points - 1)
	
	public: // construction and deletion:
		TSubspan(const TVector<vector3d<T>>& InPoints, uint32 Index)
			: Points(InPoints)
			, membership(InPoints.size())
			, members(4)
		{
			// initialize Q to the identity matrix:
			for (uint32 i = 0; i < 3; ++i)
				for (uint32 j = 0; j < 3; ++j)
					Q[i][j] = (i == j) ? 1.f : 0.f;

			members[r = 0] = Index;
			membership[Index] = true;
		}

		uint32 Size() const
		{
			return r + 1;
		}

		bool IsMember(uint32 i) const
		{
			TI_ASSERT(i < (uint32)Points.size());
			return membership[i];
		}

		uint32 GlobalIndex(uint32 i) const
		{
			TI_ASSERT(i < Size());
			return members[i];
		}

		uint32 AnyMember() const {
			TI_ASSERT(Size() > 0);
			return members[r];
		}

		//*
		void HessenbergClear(uint32 Pos)
		{
			//  clear new subdiagonal entries
			for (; Pos < r; ++Pos) 
			{
				T c, s;
				Givens(c, s, R[Pos][Pos], R[Pos][Pos + 1]);

				//  rotate partial R-rows (of the first pair, only one entry is
				//  needed, the other one is an implicit zero)
				R[Pos][Pos] = c * R[Pos][Pos] + s * R[Pos][Pos + 1];
				for (uint32 j = Pos + 1; j < r; ++j)
				{
					const T a = R[j][Pos];
					const T b = R[j][Pos + 1];
					R[j][Pos] = c * a + s * b;
					R[j][Pos + 1] = c * b - s * a;
				}

				//  rotate Q-columns
				for (uint32 i = 0; i < 3; ++i)
				{
					const T a = Q[Pos][i];
					const T b = Q[Pos + 1][i];
					Q[Pos][i] = c * a + s * b;
					Q[Pos + 1][i] = c * b - s * a;
				}
			}
		}

		void SpecialRank1Update()
		{
			//  compute w = Q^T * u
			for (uint32 i = 0; i < 3; ++i) 
			{
				W[i] = 0;
				for (uint32 k = 0; k < 3; ++k)
					W[i] += Q[i][k] * U[k];
			}

			//  rotate w down to a multiple of the first unit vector;
			//  the operations have to be recorded in R and Q
			for (uint32 k = 3 - 1; k > 0; --k)
			{
				//  k is the index of the entry to be cleared
				//  with the help of entry k-1

				//  compute Givens coefficients c,s
				T c, s;
				Givens(c, s, W[k - 1], W[k]);

				//  rotate w-entry
				W[k - 1] = c * W[k - 1] + s * W[k];

				//  rotate two R-rows;
				//  the first column has to be treated separately
				//  in order to account for the implicit zero in R[k-1][k]
				R[k - 1][k] = -s * R[k - 1][k - 1];
				R[k - 1][k - 1] *= c;
				for (uint32 j = k; j < r; ++j)
				{
					const T a = R[j][k - 1];
					const T b = R[j][k];
					R[j][k - 1] = c * a + s * b;
					R[j][k] = c * b - s * a;
				}

				//  rotate two Q-columns
				for (uint32 i = 0; i < 3; ++i)
				{
					const T a = Q[k - 1][i];
					const T b = Q[k][i];
					Q[k - 1][i] = c * a + s * b;
					Q[k][i] = c * b - s * a;
				}
			}

			//  add w * (1,...,1)^T to new R
			//  which means simply to add u[0] to each column
			//  since the other entries of u have just been eliminated
			for (uint32 j = 0; j < r; ++j)
				R[j][0] += W[0];

			//  clear subdiagonal entries
			HessenbergClear(0);
		}
		void AddPoint(int32 Index)
		{
			TI_ASSERT(!IsMember(Index));

			// compute S[i] - origin into u:
			U = Points[Index] - Points[members[r]];
			
			// appends new column u to R and updates QR-decomposition,
			// routine work with old r:
			AppendColumn();

			// move origin index and insert new index:
			membership[Index] = true;
			members[r + 1] = members[r];
			members[r] = Index;
			++r;
		}

		void RemovePoint(uint32 LocalIndex) 
		{
			TI_ASSERT(IsMember(GlobalIndex(LocalIndex)) && Size() > 1);

			membership[GlobalIndex(LocalIndex)] = false;

			if (LocalIndex == r) 
			{
				// origin must be deleted

				// We choose the right-most member of Q, i.e., column r-1 of R,
				// as the new origin.  So all relative vectors (i.e., the
				// columns of "A = QR") have to be updated by u:= old origin -
				// S[global_index(r-1)]:
				U = Points[members[r]] - Points[GlobalIndex(r - 1)];
			
				--r;

				SpecialRank1Update();

			}
			else 
			{
				// general case: delete column from R

				//  shift higher columns of R one step to the left
				vector3d<T> dummy = R[LocalIndex];
				for (uint32 j = LocalIndex + 1; j < r; ++j) {
					R[j - 1] = R[j];
					members[j - 1] = members[j];
				}
				members[r - 1] = members[r];  // shift down origin
				R[--r] = dummy;             // relink trash column

				// zero out subdiagonal entries in R
				HessenbergClear(LocalIndex);
			}
		}

		T ShortestVectorToSpan(const vector3d<T>& P, vector3d<T>& OutW)
		{
			// compute vector from p to origin, i.e., w = origin - p:
			OutW = Points[members[r]] - P;

			// remove projections of w onto the affine hull:
			for (uint32 j = 0; j < r; ++j)
			{
				const T Scale = OutW.dotProduct(Q[j]);
				OutW -= Q[j] * Scale;
			}

			return OutW.dotProduct(OutW);
		}

		T RepresentationError()
		{
			TVector<T> Lambdas(Size());
			T max = 0;
			T error;

			// cycle through all points in hull
			for (uint32 j = 0; j < Size(); ++j) 
			{
				// compute the affine representation:
				FindAffineCoefficients(Points[GlobalIndex(j)], Lambdas);

				// compare coefficient of point #j to 1.0
				error = TMath::Abs(Lambdas[j] - 1.f);
				if (error > max) max = error;

				// compare the other coefficients against 0.0
				for (uint32 i = 0; i < j; ++i) {
					error = TMath::Abs(Lambdas[i] - 0.f);
					if (error > max) max = error;
				}
				for (uint32 i = j + 1; i < Size(); ++i) {
					error = TMath::Abs(Lambdas[i] - 0.f);
					if (error > max) max = error;
				}
			}

			return max;
		}

		void FindAffineCoefficients(const vector3d<T>& P, TVector<T>& Lambdas)
		{
			// compute relative position of p, i.e., u = p - origin:
			U = P - Points[members[r]];

			// calculate Q^T u into w:
			W = vector3d<T>();
			for (uint32 i = 0 ; i < 3 ; ++ i)
			{
				W[i] += Q[i].dotProduct(U);
			}

			// We compute the coefficients by backsubstitution.  Notice that
			//
			//     c = \sum_{i\in M} \lambda_i (S[i] - origin)
			//       = \sum_{i\in M} \lambda_i S[i] + (1-s) origin
			//
			// where s = \sum_{i\in M} \lambda_i.-- We compute the coefficient
			// (1-s) of the origin in the variable origin_lambda:
			T OriginLambda = 1.f;
			for (int32 j = r - 1; j >= 0; --j)
			{
				for (uint32 k = j + 1; k < r; ++k)
				{
					W[j] -= Lambdas[k] * R[k][j];
				}
				OriginLambda -= Lambdas[j] = W[j] / R[j][j];
			}
			// The r-th coefficient corresponds to the origin (cf. remove_point()):
			Lambdas[r] = OriginLambda;
		}

		void AppendColumn()
		{
			TI_ASSERT(r < 3);

			//  compute new column R[r] = Q^T * u
			for (uint32 i = 0; i < 3; ++i) {
				R[r][i] = 0;
				for (uint32 k = 0; k < 3; ++k)
					R[r][i] += Q[i][k] * U[k];
			}

			//  zero all entries R[r][dim-1] down to R[r][r+1]
			for (uint32 j = 3 - 1; j > r; --j) 
			{
				//  j is the index of the entry to be cleared
				//  with the help of entry j-1

				//  compute Givens coefficients c,s
				T c, s;
				Givens(c, s, R[r][j - 1], R[r][j]);

				//  rotate one R-entry (the other one is an implicit zero)
				R[r][j - 1] = c * R[r][j - 1] + s * R[r][j];

				//  rotate two Q-columns
				for (uint32 i = 0; i < 3; ++i) {
					const T a = Q[j - 1][i];
					const T b = Q[j][i];
					Q[j - 1][i] = c * a + s * b;
					Q[j][i] = c * b - s * a;
				}
			}
		}
	};
	TSmallestEncloseSphere(const TVector<vector3d<T>>& InPoints);
	~TSmallestEncloseSphere();

	T GetRadius() const
	{
		return Radius;
	}

	const vector3d<T>& GetCenter() const
	{
		return Center;
	}

private:
	bool SuccessfulDrop();
	T FindStopFraction(int32& stopper);

private:
	const TVector<vector3d<T>>& Points;
	vector3d<T> Center;
	TVector<T> Lambdas;
	TSubspan * Support;
	vector3d<T> CenterToAff;
	vector3d<T> CenterToPoint;
	T RadiusSquare, Radius;
	T DistToAff, DistToAffSquare;
};

template<typename T>
const T TSmallestEncloseSphere<T>::Eps = T(1e-14);

template<typename T>
TSmallestEncloseSphere<T>::TSmallestEncloseSphere(const TVector<vector3d<T>>& InPoints)
	: Points(InPoints)
{
	TI_ASSERT(InPoints.size() > 0);
	Center = InPoints[0];
	Lambdas.resize(4);

	const uint32 NPoints = (uint32)InPoints.size();
	// find farthest point:
	RadiusSquare = 0.f;
	uint32 Farest = 0;
	for (uint32 j = 1; j < NPoints; ++j)
	{
		T Dist = (InPoints[j] - Center).getLengthSQ();

		if (Dist >= RadiusSquare)
		{
			RadiusSquare = Dist;
			Farest = j;
		}
	}
	Radius = sqrt(RadiusSquare);

	Support = ti_new TSubspan(InPoints, Farest);

	uint32 Iteration = 0;
	while (true)
	{
		// Compute a walking direction and walking vector,
		// and check if the former is perhaps too small:
		while ((DistToAff = sqrt(DistToAffSquare = Support->ShortestVectorToSpan(Center, CenterToAff))) <= Eps * Radius)
		{
			// We are closer than Eps * radius_square, so we try a drop:
			if (!SuccessfulDrop())
			{
				// If that is not possible, the center lies in the convex hull
				// and we are done.
				return;
			}
		}

		// determine how far we can walk in direction center_to_aff
		// without losing any point ('stopper', say) in S:
		int32 stopper;
		T scale = FindStopFraction(stopper);

		// Note: In theory, the following if-statement should simply read
		//
		//  if (stopper >= 0) {
		//    // ...
		//
		// However, due to rounding errors, it may happen in practice that
		// stopper is nonnegative and the Support is already full (see #14);
		// in this casev we cannot add yet another point to the Support.
		//
		// Therefore, the condition reads:
		if (stopper >= 0 && Support->Size() <= 3) {
			// stopping point exists

			// walk as far as we can
			Center += CenterToAff * scale;
			//for (uint32 i = 0; i < 3; ++i)
			//	center[i] += scale * center_to_aff[i];

			// update the radius
			const vector3d<T>& stop_point = Points[Support->AnyMember()];
			RadiusSquare = 0.f;
			RadiusSquare += (stop_point - Center).getLengthSQ();
			Radius = sqrt(RadiusSquare);

			// and add stopper to Support
			Support->AddPoint(stopper);
		}
		else
		{
			//  we can run unhindered into the affine hull
			Center += CenterToAff;

			// update the radius:
			const vector3d<T>& stop_point = Points[Support->AnyMember()];
			RadiusSquare = 0.f;
			RadiusSquare += (stop_point - Center).getLengthSQ();
			Radius = sqrt(RadiusSquare);

			// Theoretically, the distance to the affine hull is now zero
			// and we would thus drop a point in the next iteration.
			// For numerical stability, we don't rely on that to happen but
			// try to drop a point right now:
			if (!SuccessfulDrop())
			{
				// Drop failed, so the center lies in conv(Support) and is thus
				// optimal.
				return;
			}
		}

		++Iteration;
		//if (Iteration > Points.size())
		//{
		//	bool print_points = false;
		//	if (print_points)
		//	{
		//		for (uint32 cp = 0; cp < (uint32)Points.size(); ++cp)
		//		{
		//			const vector3df64& cpp = Points[cp];
		//			printf("vector3df64(%ff, %ff, %ff),\n", cpp.X, cpp.Y, cpp.Z);
		//		}
		//	}
		//}
	}
}

template<typename T>
TSmallestEncloseSphere<T>::~TSmallestEncloseSphere()
{
	if (Support != nullptr)
	{
		ti_delete Support;
	}
}

template<typename T>
bool TSmallestEncloseSphere<T>::SuccessfulDrop()
{
	// find coefficients of the affine combination of center:
	Support->FindAffineCoefficients(Center, Lambdas);

	// find a non-positive coefficient:
	uint32 smallest = 0; // Note: assignment prevents compiler warnings.
	T minimum(1);
	for (uint32 i = 0; i < Support->Size(); ++i)
		if (Lambdas[i] < minimum) {
			minimum = Lambdas[i];
			smallest = i;
		}

	// drop a point with non-positive coefficient, if any:
	if (minimum <= 0)
	{
		Support->RemovePoint(smallest);
		return true;
	}
	return false;
}

template<typename T>
T TSmallestEncloseSphere<T>::FindStopFraction(int32& stopper)
{
	// We would like to walk the full length of center_to_aff ...
	T scale = 1.f;
	stopper = -1;

	T margin = 0.f;

		// ... but one of the points in S might hinder us:
	for (uint32 j = 0; j < (uint32)Points.size(); ++j)
	{
		if (!Support->IsMember(j)) {

			// compute vector center_to_point from center to the point S[i]:
			CenterToPoint = Points[j] - Center;

			const T DirPointProd = CenterToAff.dotProduct(CenterToPoint);

			// we can ignore points beyond Support since they stay
			// enclosed anyway:
			if (DistToAffSquare - DirPointProd
				// make new variable 'radius_times_dist_to_aff'? !
				< Eps * Radius * DistToAff)
				continue;

			// compute the fraction we can walk along center_to_aff until
			// we hit point S[i] on the boundary:
			// (Better don't try to understand this calculus from the code,
			//  it needs some pencil-and-paper work.)
			T bound = RadiusSquare;
			bound -= CenterToPoint.dotProduct(CenterToPoint);
			//	inner_product(center_to_point, center_to_point + dim,
			//	center_to_point, float(0));
			bound /= 2 * (DistToAffSquare - DirPointProd);

			// watch for numerical instability - if bound=0 then we are
			// going to walk by zero units, and thus hit an infinite loop.
			//
			// If we are walking < 0, then we are going the wrong way,
			// which can happen if we were to disable the test just above
			// (the "dist_to_aff_square-dir_point_prod" test)

			// take the smallest fraction:
			if (bound > 0 && bound < scale) {
				scale = bound;
				stopper = j;
				//SEB_DEBUG(margin = dist_to_aff - DirPointProd / dist_to_aff;)
				margin = DistToAff - DirPointProd / DistToAff;
			}
		}
	}

	return scale;
}