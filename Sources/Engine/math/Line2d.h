
#pragma once
#ifndef _LINE_2D_H_INCLUDED__
#define _LINE_2D_H_INCLUDED__

namespace tix
{

//! 3D line between two points with intersection methods.
template <class T>
class line2d
{
public:

	//! Default constructor
	/** line from (0,0,0) to (1,1,1) */
	line2d() : start(0,0), end(1,1) {}
	//! Constructor with two points
	line2d(T xa, T ya, T xb, T yb) : start(xa, ya), end(xb, yb) {}

	// Line-Line Intersection
	bool isIntersectWithLine2d(const line2d<T>& other)
	{
		// algorithm from
		// http://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
		T s1_x, s1_y, s2_x, s2_y;
		s1_x = end.X - start.X;
		s1_y = end.Y - start.Y;
		s2_x = other.end.X - other.start.X;
		s2_y = other.end.Y - other.start.Y;

		float s, t;
		float det		= (float)(-s2_x * s1_y + s1_x * s2_y);
		s = (-s1_y * (start.X - other.start.X) + s1_x * (start.Y - other.start.Y)) / det;
		t = ( s2_x * (start.Y - other.start.Y) - s2_y * (start.X - other.start.X)) / det;

		if (s >= 0 && s <= 1 && t >= 0 && t <= 1)
		{
			// Collision detected
			//if (i_x != NULL)
			//	*i_x = p0_x + (t * s1_x);
			//if (i_y != NULL)
			//	*i_y = p0_y + (t * s1_y);
			return true;
		}

		return false; // No collision

		// algorithm from 
		// http://community.topcoder.com/tc?module=Static&d1=tutorials&d2=geometry2
		//T a0, a1, b0, b1, c0, c1;
		//T x, y;
		//a0		= end.Y - start.Y;
		//b0		= start.X - end.X;
		//c0		= a0 * start.X + b0 * start.Y;

		//a1		= other.end.Y - other.start.Y;
		//b1		= other.start.X - other.end.X;
		//c1		= a1 * other.start.X + b1 * other.start.Y;

		//T det	= a0 * b1 - a1 * b0;
		//if (det == 0)
		//{
		//	return false;
		//}
		//else
		//{
		//	x		= (b1 * c0 - b0 * c1) / det;
		//	y		= (a0 * c1 - a1 * c0) / det;

		//	vector2d<T> v0, v1, v2, v3;
		//	v0.X	= start.X - x;
		//	v0.Y	= start.Y - y;
		//	v1.X	= end.X - x;
		//	v1.Y	= end.Y - y;
		//	v2.X	= other.start.X - x;
		//	v2.Y	= other.start.Y - y;
		//	v3.X	= other.end.X - x;
		//	v3.Y	= other.end.Y - y;

		//	T l0	= (start - end).getLengthSQ();
		//	T l1	= (other.start - other.end).getLengthSQ();

		//	return	v0.getLengthSQ() <= l0 &&
		//			v1.getLengthSQ() <= l0 &&
		//			v2.getLengthSQ() <= l1 &&
		//			v3.getLengthSQ() <= l1;
		//}
	}


	// member variables

	//! Start point of line
	vector2d<T> start;
	//! End point of line
	vector2d<T> end;
};

//! Typedef for an f32 line.
typedef line2d<f32> line2df;
//! Typedef for an integer line.
typedef line2d<int32> line2di;

} // end namespace ti

#endif

