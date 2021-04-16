/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TFdmIccgSolver.h"

const float tolerance = std::numeric_limits<float>::epsilon();
const uint32 maxNumberOfIterations = 100;

inline void BlasResidual(const TArray3<FdmMatrixRow3>& a, const TArray3<float>& x,
					const TArray3<float>& b, TArray3<float>& r)
{
	const vector3di& Size = r.GetSize();

	// x[] always be Zero, so this is equal to r = b ???
	//r = b;
	for (int32 k = 0; k < Size.Z; k++)
	{
		for (int32 j = 0; j < Size.Y; j++)
		{
			for (int32 i = 0; i < Size.X; i++)
			{
				int32 Index = r.GetAccessIndex(i, j, k);
				int32 IndexX1 = r.GetAccessIndex(i - 1, j, k);
				int32 IndexX2 = r.GetAccessIndex(i + 1, j, k);
				int32 IndexY1 = r.GetAccessIndex(i, j - 1, k);
				int32 IndexY2 = r.GetAccessIndex(i, j + 1, k);
				int32 IndexZ1 = r.GetAccessIndex(i, j, k - 1);
				int32 IndexZ2 = r.GetAccessIndex(i, j, k + 1);

				r[Index] = b[Index] - a[Index].Center * x[Index] -
					((i > 0) ? a[IndexX1].Right * x[IndexX1] : 0.f) -
					((i + 1 < Size.X) ? a[Index].Right * x[IndexX2] : 0.f) -
					((j > 0) ? a[IndexY1].Up * x[IndexY1] : 0.f) -
					((j + 1 < Size.Y) ? a[Index].Up * x[IndexY2] : 0.f) -
					((k > 0) ? a[IndexZ1].Front * x[IndexZ1] : 0.f) -
					((k + 1 < Size.Z) ? a[Index].Front * x[IndexZ2] : 0.f);
			}
		}
	}
}

inline float BlasDot(const TArray3<float>& a, const TArray3<float>& b)
{
	const vector3di& Size = a.GetSize();
	TI_ASSERT(a.GetSize() == b.GetSize());

	float Result = 0.f;
	for (int32 _z = Size.Z - 1; _z >= 0; _z--)
	{
		for (int32 _y = Size.Y - 1; _y >= 0; _y--)
		{
			for (int32 _x = Size.X - 1; _x >= 0; _x--)
			{
				int32 Index = a.GetAccessIndex(_x, _y, _z);
				Result += a[Index] * b[Index];
			}
		}
	}
	return Result;
}

inline void BlasMvm(const TArray3<FdmMatrixRow3>& m, const TArray3<float>& v, TArray3<float>& Result)
{
	const vector3di& Size = m.GetSize();

	for (int32 k = 0; k < Size.Z; k++)
	{
		for (int32 j = 0; j < Size.Y; j++)
		{
			for (int32 i = 0; i < Size.X; i++)
			{
				int32 Index = m.GetAccessIndex(i, j, k);
				int32 IndexX1 = m.GetAccessIndex(i - 1, j, k);
				int32 IndexX2 = m.GetAccessIndex(i + 1, j, k);
				int32 IndexY1 = m.GetAccessIndex(i, j - 1, k);
				int32 IndexY2 = m.GetAccessIndex(i, j + 1, k);
				int32 IndexZ1 = m.GetAccessIndex(i, j, k - 1);
				int32 IndexZ2 = m.GetAccessIndex(i, j, k + 1);
				Result[Index] =
					m[Index].Center * v[Index] +
					((i > 0) ? m[IndexX1].Right * v[IndexX1] : 0.f) +
					((i < Size.X) ? m[Index].Right * v[IndexX2] : 0.f) +
					((j > 0) ? m[IndexY1].Up * v[IndexY1] : 0.f) +
					((j < Size.Y) ? m[Index].Up * v[IndexY2] : 0.f) +
					((k > 0) ? m[IndexZ1].Front * v[IndexZ1] : 0.f) +
					((k < Size.Z) ? m[Index].Front * v[IndexZ2] : 0.f);
			}
		}
	}
}

inline void BlasAxpy(float a, const TArray3<float>& x, const TArray3<float>& y, TArray3<float>& Result)
{
	const vector3di& Size = x.GetSize();

	for (int32 k = 0; k < Size.Z; k++)
	{
		for (int32 j = 0; j < Size.Y; j++)
		{
			for (int32 i = 0; i < Size.X; i++)
			{
				int32 Index = x.GetAccessIndex(i, j, k);
				Result[Index] = a * x[Index] + y[Index];
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////

TFdmIccgSolver::TFdmIccgSolver(const vector3di& InSize)
	: Size(InSize)
{
}

TFdmIccgSolver::~TFdmIccgSolver()
{
}

void TFdmIccgSolver::Solve(const TArray3<FdmMatrixRow3>& A, const TArray3<float>& b, TArray3<float>& x)
{
	_r.Resize(Size);
	_d.Resize(Size);
	_q.Resize(Size);
	_s.Resize(Size);

	x.ResetZero();
	_r.ResetZero();
	_d.ResetZero();
	_q.ResetZero();
	_s.ResetZero();

	_precond.Build(A, Size);

	// pcg ?? what does this mean? Predict cg?
	{
		// r = b - Ax
		BlasResidual(A, x, b, _r);

		// d = M^-1r
		_precond.Solve(A, _r, _d);

		// sigmaNew = r.d
		float sigmaNew = BlasDot(_r, _d);

		uint32 iter = 0;
		bool trigger = false;
		while (sigmaNew > TMath::Sqr(tolerance) && iter < maxNumberOfIterations) {
			// q = Ad
			BlasMvm(A, _d, _q);

			// alpha = sigmaNew/d.q
			float alpha = sigmaNew / BlasDot(_d, _q);

			// x = x + alpha*d
			BlasAxpy(alpha, _d, x, x);

			// if i is divisible by 50...
			if (trigger || (iter % 50 == 0 && iter > 0)) {
				// r = b - Ax
				BlasResidual(A, x, b, _r);
				trigger = false;
			}
			else {
				// r = r - alpha*q
				BlasAxpy(-alpha, _q, _r, _r);
			}

			// s = M^-1r
			_precond.Solve(A, _r, _s);

			// sigmaOld = sigmaNew
			float sigmaOld = sigmaNew;

			// sigmaNew = r.s
			sigmaNew = BlasDot(_r, _s);

			if (sigmaNew > sigmaOld) 
			{
				trigger = true;
			}

			// beta = sigmaNew/sigmaOld
			float beta = sigmaNew / sigmaOld;

			// d = s + beta*d
			BlasAxpy(beta, _d, _s, _d);

			++iter;
		}
	}
}


void TFdmIccgSolver::Preconditioner::Build(const TArray3<FdmMatrixRow3>& matrix, const vector3di& Size)
{
	d.Resize(Size);
	y.Resize(Size);
	d.ResetZero();
	y.ResetZero();
	const TArray3<FdmMatrixRow3>& A = matrix;

	for (int32 k = 0; k < Size.Z; k++)
	{
		for (int32 j = 0; j < Size.Y; j++)
		{
			for (int32 i = 0; i < Size.X; i++)
			{
				int32 Index = A.GetAccessIndex(i, j, k);
				int32 IndexX1 = A.GetAccessIndex(i - 1, j, k);
				int32 IndexY1 = A.GetAccessIndex(i, j - 1, k);
				int32 IndexZ1 = A.GetAccessIndex(i, j, k - 1);

				float denom = A[Index].Center -
					((i > 0) ? TMath::Sqr(A[IndexX1].Right) * d[IndexX1] : 0.f) -
					((j > 0) ? TMath::Sqr(A[IndexY1].Up) * d[IndexY1] : 0.f) -
					((k > 0) ? TMath::Sqr(A[IndexZ1].Front) * d[IndexZ1] : 0.f);

				if (TMath::Abs(denom) > 0.f)
				{
					d[Index] = 1.f / denom;
				}
				else
				{
					d[Index] = 0.f;
				}
			}
		}
	}
}

void TFdmIccgSolver::Preconditioner::Solve(const TArray3<FdmMatrixRow3>& A, const TArray3<float>& b, TArray3<float>& x)
{
	const vector3di& Size = A.GetSize();
	for (int32 k = 0; k < Size.Z; k++)
	{
		for (int32 j = 0; j < Size.Y; j++)
		{
			for (int32 i = 0; i < Size.X; i++)
			{
				int32 Index = A.GetAccessIndex(i, j, k);
				int32 IndexX1 = A.GetAccessIndex(i - 1, j, k);
				int32 IndexY1 = A.GetAccessIndex(i, j - 1, k);
				int32 IndexZ1 = A.GetAccessIndex(i, j, k - 1);

				y[Index] =
					(b[Index] -
						((i > 0) ? A[IndexX1].Right * y[IndexX1] : 0.f) -
						((j > 0) ? A[IndexY1].Up * y[IndexY1] : 0.f) -
						((k > 0) ? A[IndexZ1].Front * y[IndexZ1] : 0.f)) *
					d[Index];
			}
		}
	}

	for (int32 k = Size.Z - 1; k >= 0; k--)
	{
		for (int32 j = Size.Y - 1; j >= 0; j--)
		{
			for (int32 i = Size.X - 1; i >= 0; i--)
			{
				int32 Index = A.GetAccessIndex(i, j, k);
				int32 IndexX1 = A.GetAccessIndex(i + 1, j, k);
				int32 IndexY1 = A.GetAccessIndex(i, j + 1, k);
				int32 IndexZ1 = A.GetAccessIndex(i, j, k + 1);

				x[Index] =
					y[Index] -
					((i + 1 < Size.X) ? A[Index].Right * x[IndexX1] : 0.f) -
					((j + 1 < Size.Y) ? A[Index].Up * x[IndexY1] : 0.f) -
					((k + 1 < Size.Z) ? A[Index].Front * x[IndexZ1] : 0.f) *
					d[Index];
			}
		}
	}

}