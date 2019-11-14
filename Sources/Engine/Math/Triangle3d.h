// Copyright (C) 2002-2010 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __IRR_TRIANGLE_3D_H_INCLUDED__
#define __IRR_TRIANGLE_3D_H_INCLUDED__

namespace tix
{

	//! 3d triangle template class for doing collision detection and other things.
	template <class T>
	class triangle3d
	{
	public:

		//! Constructor for an all 0 triangle
		triangle3d() {}
		//! Constructor for triangle with given three vertices
		triangle3d(vector3d<T> v1, vector3d<T> v2, vector3d<T> v3) : pointA(v1), pointB(v2), pointC(v3) {}

		//! Equality operator
		bool operator==(const triangle3d<T>& other) const
		{
			return other.pointA==pointA && other.pointB==pointB && other.pointC==pointC;
		}

		//! Inequality operator
		bool operator!=(const triangle3d<T>& other) const
		{
			return !(*this==other);
		}

		//! Determines if the triangle is totally inside a bounding box.
		/** \param box Box to check.
		\return True if triangle is within the box, otherwise false. */
		bool isTotalInsideBox(const aabbox3d<T>& box) const
		{
			return (box.isPointInside(pointA) &&
				box.isPointInside(pointB) &&
				box.isPointInside(pointC));
		}

		//! Determines if the triangle is totally outside a bounding box.
		/** \param box Box to check.
		\return True if triangle is outside the box, otherwise false. */
		bool isTotalOutsideBox(const aabbox3d<T>& box) const
		{
			return ((pointA.X > box.MaxEdge.X && pointB.X > box.MaxEdge.X && pointC.X > box.MaxEdge.X) ||

				(pointA.Y > box.MaxEdge.Y && pointB.Y > box.MaxEdge.Y && pointC.Y > box.MaxEdge.Y) ||
				(pointA.Z > box.MaxEdge.Z && pointB.Z > box.MaxEdge.Z && pointC.Z > box.MaxEdge.Z) ||
				(pointA.X < box.MinEdge.X && pointB.X < box.MinEdge.X && pointC.X < box.MinEdge.X) ||
				(pointA.Y < box.MinEdge.Y && pointB.Y < box.MinEdge.Y && pointC.Y < box.MinEdge.Y) ||
				(pointA.Z < box.MinEdge.Z && pointB.Z < box.MinEdge.Z && pointC.Z < box.MinEdge.Z));
		}

		//! Determines if the triangle is intersect with a bounding box
		// For any two convex meshes, to find whether they intersect, 
		// you need to check if there exist a separating plane.
		// If it does, they do not intersect.The plane can be picked from 
		// any face of either shape, or the edge cross - products.
		// The plane is defined as a normal and an offset from Origo.
		// So, you only have to check three faces of the AABB, and one face of the triangle.
		//https://stackoverflow.com/questions/17458562/efficient-aabb-triangle-intersection-in-c-sharp
		/** \param box Box to check.
		\return True if triangle is intersect the box, otherwise false. */
		bool isIntersectWithBox(const aabbox3d<T>& box) const
		{
			T TriangleMin, TriangleMax;
			T BoxMin, BoxMax;

			// Test the box normals (x-, y- and z-axes)
			vector3d<T> BoxNormals[3] =
			{
				vector3d<T>(1,0,0),
				vector3d<T>(0,1,0),
				vector3d<T>(0,0,1)
			};

			auto ProjectTriangle = [](const triangle3d<T>& Tri, const vector3d<T>& Axis, T& MinValue, T& MaxValue)
			{
				MinValue = FLT_MAX;
				MaxValue = FLT_MIN;

				const vector3d<T> Points[] =
				{
					Tri.pointA,
					Tri.pointB,
					Tri.pointC
				};

				for (int32 i = 0; i < 3; ++i)
				{
					T V = Axis.dotProduct(Points[i]);
					if (V < MinValue)
						MinValue = V;
					if (V > MaxValue)
						MaxValue = V;
				}
			};
			auto ProjectBox = [](const aabbox3d<T>& Box, const vector3d<T>& Axis, T& MinValue, T& MaxValue)
			{
				MinValue = FLT_MAX;
				MaxValue = FLT_MIN;

				const vector3d<T> Points[] =
				{
					vector3d<T>(Box.MinEdge.X, Box.MinEdge.Y, Box.MinEdge.Z),
					vector3d<T>(Box.MaxEdge.X, Box.MinEdge.Y, Box.MinEdge.Z),
					vector3d<T>(Box.MinEdge.X, Box.MaxEdge.Y, Box.MinEdge.Z),
					vector3d<T>(Box.MaxEdge.X, Box.MaxEdge.Y, Box.MinEdge.Z),

					vector3d<T>(Box.MinEdge.X, Box.MinEdge.Y, Box.MaxEdge.Z),
					vector3d<T>(Box.MaxEdge.X, Box.MinEdge.Y, Box.MaxEdge.Z),
					vector3d<T>(Box.MinEdge.X, Box.MaxEdge.Y, Box.MaxEdge.Z),
					vector3d<T>(Box.MaxEdge.X, Box.MaxEdge.Y, Box.MaxEdge.Z)
				};

				for (int32 i = 0; i < 8; ++i)
				{
					T V = Axis.dotProduct(Points[i]);
					if (V < MinValue)
						MinValue = V;
					if (V > MaxValue)
						MaxValue = V;
				}
			};
			for (int32 i = 0; i < 3; i++)
			{
				const vector3d<T>& N = BoxNormals[i];
				ProjectTriangle(*this, BoxNormals[i], TriangleMin, TriangleMax);
				if (TriangleMax < box.MinEdge[i] || TriangleMin > box.MaxEdge[i])
					return false; // No intersection possible.
			}

			// Test the triangle normal
			vector3d<T> TriN = getNormal().normalize();
			T TriangleOffset = TriN.dotProduct(pointA);
			ProjectBox(box, TriN, BoxMin, BoxMax);
			if (BoxMax < TriangleOffset || BoxMin > TriangleOffset)
				return false; // No intersection possible.

			// Test the nine edge cross-products
			vector3d<T> TriangleEdges[] =
			{
				pointA - pointB,
				pointB - pointC,
				pointC - pointA
			};
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					// The box normals are the same as it's edge tangents
					vector3d<T> Axis = TriangleEdges[i].crossProduct(BoxNormals[j]);
					ProjectBox(box, Axis, BoxMin, BoxMax);
					ProjectTriangle(*this, Axis, TriangleMin, TriangleMax);
					if (BoxMax <= TriangleMin || BoxMin >= TriangleMax)
						return false; // No intersection possible
				}
			}

			// No separating axis found.
			return true;
		}

		//! Get the closest point on a triangle to a point on the same plane.
		/** \param p Point which must be on the same plane as the triangle.
		\return The closest point of the triangle */
		vector3d<T> closestPointOnTriangle(const vector3d<T>& p) const
		{
			const vector3d<T> rab = line3d<T>(pointA, pointB).getClosestPoint(p);
			const vector3d<T> rbc = line3d<T>(pointB, pointC).getClosestPoint(p);
			const vector3d<T> rca = line3d<T>(pointC, pointA).getClosestPoint(p);

			const T d1 = rab.getDistanceFrom(p);
			const T d2 = rbc.getDistanceFrom(p);
			const T d3 = rca.getDistanceFrom(p);

			if (d1 < d2)
				return d1 < d3 ? rab : rca;

			return d2 < d3 ? rbc : rca;
		}

		//! Check if a point is inside the triangle (border-points count also as inside)
		/** NOTE: When working with T='int' you should prefer isPointInsideFast, as 
		isPointInside will run into number-overflows already with coordinates in the 3-digit-range.
		\param p Point to test. Assumes that this point is already
		on the plane of the triangle.
		\return True if the point is inside the triangle, otherwise false. */
		bool isPointInside(const vector3d<T>& p) const
		{
			return (isOnSameSide(p, pointA, pointB, pointC) &&
 				isOnSameSide(p, pointB, pointA, pointC) &&
 				isOnSameSide(p, pointC, pointA, pointB));
		}

		//! Check if a point is inside the triangle (border-points count also as inside)
		/** This method uses a barycentric coordinate system. 
		It is faster than isPointInside but is more susceptible to floating point rounding 
		errors. This will especially be noticable when the FPU is in single precision mode 
		(which is for example set on default by Direct3D).
		\param p Point to test. Assumes that this point is already
		on the plane of the triangle.
		\return True if point is inside the triangle, otherwise false. */
		bool isPointInsideFast(const vector3d<T>& p) const
		{
			const vector3d<T> a = pointC - pointA;
			const vector3d<T> b = pointB - pointA;
			const vector3d<T> c = p - pointA;
			
			const float64 dotAA = a.dotProduct( a);
			const float64 dotAB = a.dotProduct( b);
			const float64 dotAC = a.dotProduct( c);
			const float64 dotBB = b.dotProduct( b);
			const float64 dotBC = b.dotProduct( c);
			 
			// get coordinates in barycentric coordinate system
			const float64 invDenom =  1/(dotAA * dotBB - dotAB * dotAB); 
			const float64 u = (dotBB * dotAC - dotAB * dotBC) * invDenom;
			const float64 v = (dotAA * dotBC - dotAB * dotAC ) * invDenom;
		 
			// We count border-points as inside to keep downward compatibility.
			// That's why we use >= and <= instead of > and < as more commonly seen on the web.
			return (u >= 0) && (v >= 0) && (u + v <= 1);

		}


		//! Get an intersection with a 3d line.
		/** \param line Line to intersect with.
		\param outIntersection Place to store the intersection point, if there is one.
		\return True if there was an intersection, false if not. */
		bool getIntersectionWithLimitedLine(const line3d<T>& line,
			vector3d<T>& outIntersection) const
		{
			return getIntersectionWithLine(line.start,
				line.getVector(), outIntersection) &&
				outIntersection.isBetweenPoints(line.start, line.end);
		}


		//! Get an intersection with a 3d line.
		/** Please note that also points are returned as intersection which
		are on the line, but not between the start and end point of the line.
		If you want the returned point be between start and end
		use getIntersectionWithLimitedLine().
		\param linePoint Point of the line to intersect with.
		\param lineVect Vector of the line to intersect with.
		\param outIntersection Place to store the intersection point, if there is one.
		\return True if there was an intersection, false if there was not. */
		bool getIntersectionWithLine(const vector3d<T>& linePoint,
			const vector3d<T>& lineVect, vector3d<T>& outIntersection) const
		{
			if (getIntersectionOfPlaneWithLine(linePoint, lineVect, outIntersection))
				return isPointInside(outIntersection);

			return false;
		}

		bool getIntersectionWithLine( const vector3d<T>& linePoint, 
			const vector3d<T>& lineVect, float* t, float* u, float* v) const
		{
			// Find vectors for two edges sharing vert0
			vector3d<T> edge1 = pointB - pointA;
			vector3d<T> edge2 = pointC - pointA;

			// Begin calculating determinant - also used to calculate U parameter
			vector3d<T> pvec = lineVect.crossProduct(edge2);

			// If determinant is near zero, ray lies in plane of triangle
			float32 det = edge1.dotProduct(pvec);

			vector3d<T> tvec;
			if( det > 0 )
			{
				tvec = linePoint - pointA;
			}
			else
			{
				tvec = pointA - linePoint;
				det = -det;
			}

			if( det < 0.0001f )
				return false;

			// Calculate U parameter and test bounds
			*u = tvec.dotProduct(pvec);
			if( *u < 0.0f || *u > det )
				return false;

			// Prepare to test V parameter
			vector3d<T> qvec = tvec.crossProduct(edge1);

			// Calculate V parameter and test bounds
			*v = lineVect.dotProduct(qvec);
			if( *v < 0.0f || *u + *v > det )
				return false;

			// Calculate t, scale parameters, ray intersects triangle
			*t = edge2.dotProduct(qvec);
			float32 fInvDet = 1.0f / det;
			*t *= fInvDet;
			*u *= fInvDet;
			*v *= fInvDet;

			return true;
		}


		//! Calculates the intersection between a 3d line and the plane the triangle is on.
		/** \param lineVect Vector of the line to intersect with.
		\param linePoint Point of the line to intersect with.
		\param outIntersection Place to store the intersection point, if there is one.
		\return True if there was an intersection, else false. */
		bool getIntersectionOfPlaneWithLine(const vector3d<T>& linePoint,
			const vector3d<T>& lineVect, vector3d<T>& outIntersection) const
		{
			const vector3d<T> normal = getNormal().normalize();
			T t2;

			if ( iszero ( t2 = normal.dotProduct(lineVect) ) )
				return false;

			T d = pointA.dotProduct(normal);
			T t = -(normal.dotProduct(linePoint) - d) / t2;
			outIntersection = linePoint + (lineVect * t);
			return true;
		}


		//! Get the normal of the triangle.
		/** Please note: The normal is not always normalized. */
		vector3d<T> getNormal() const
		{
			return (pointB - pointA).crossProduct(pointC - pointA);
		}

		//! Test if the triangle would be front or backfacing from any point.
		/** Thus, this method assumes a camera position from which the
		triangle is definitely visible when looking at the given direction.
		Do not use this method with points as it will give wrong results!
		\param lookDirection Look direction.
		\return True if the plane is front facing and false if it is backfacing. */
		bool isFrontFacing(const vector3d<T>& lookDirection) const
		{
			const vector3d<T> n = getNormal().normalize();
			const float32 d = (float32)n.dotProduct(lookDirection);
			return F32_LOWER_EQUAL_0(d);
		}

		//! Get the plane of this triangle.
		plane3d<T> getPlane() const
		{
			return plane3d<T>(pointA, pointB, pointC);
		}

		//! Get the bounding box of this triangle
		aabbox3d<T> getBoundingBox() const
		{
			aabbox3d<T> Box(pointA);
			Box.addInternalPoint(pointB);
			Box.addInternalPoint(pointC);
			return Box;
		}

		//! Get the area of the triangle
		T getArea() const
		{
			return (pointB - pointA).crossProduct(pointC - pointA).getLength() * 0.5f;

		}

		T getArea1() const
		{
			float a = (pointB - pointA).getLength();
			float b = (pointC - pointA).getLength();
			float c = (pointB - pointC).getLength();

			float p = (a + b + c) * 0.5f;
			float s = sqrt(p * (p-a) * (p-b) * (p-c));

			return s;
		}

		//! sets the triangle's points
		void set(const vector3d<T>& a, const vector3d<T>& b, const vector3d<T>& c)
		{
			pointA = a;
			pointB = b;
			pointC = c;
		}

		//! the three points of the triangle
		vector3d<T> pointA;
		vector3d<T> pointB;
		vector3d<T> pointC;

	private:
		bool isOnSameSide(const vector3d<T>& p1, const vector3d<T>& p2,
			const vector3d<T>& a, const vector3d<T>& b) const
		{
			vector3d<T> bminusa = b - a;
			vector3d<T> cp1 = bminusa.crossProduct(p1 - a);
			vector3d<T> cp2 = bminusa.crossProduct(p2 - a);
			return (cp1.dotProduct(cp2) >= 0.0f);
		}
	};


	//! Typedef for a float32 3d triangle.
	typedef triangle3d<float32> triangle3df;

	//! Typedef for an integer 3d triangle.
	typedef triangle3d<int32> triangle3di;

} // end namespace ti

#endif

