//-*-c++-*-
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#pragma once
#ifndef __IRR_RECT_H_INCLUDED__
#define __IRR_RECT_H_INCLUDED__

namespace tix
{
	//! Rectangle template.
	/** Mostly used by 2D GUI elements and for 2D drawing methods.
		It has 2 positions instead of position and dimension and a fast
		method for collision detection with other rectangles and points.
	*/
	template <class T>
	class rect
	{
	public:

		rect() : Upper(0), Left(0), Lower(0), Right(0) {}

		rect(T x, T y, T x2, T y2)
			: Upper(y), Left(x), Lower(y2), Right(x2) {}

		void reset(T x, T y)
		{
			Left	= x;
			Right	= x;
			Upper	= y;
			Lower	= y;
		}

		void reset(const vector2d<T>& p)
		{
			reset(p.X, p.Y);
		}

		bool operator==(const rect<T>& other) const
		{
			return (Upper == other.Upper &&
					Left == other.Left &&
					Lower == other.Lower &&
					Right == other.Right);
		}


		bool operator!=(const rect<T>& other) const
		{
			return (Upper != other.Upper ||
					Left != other.Left ||
					Lower != other.Lower ||
					Right != other.Right);
		}

		rect<T>& operator+=(const rect<T>& other)
		{
			addInternalPoint(other.Upper, other.Left);
			addInternalPoint(other.Lower, other.Right);
			return *this;
		}

		rect<T> operator * (T num) const
		{
			rect<T> rc;
			rc.Left			= this->Left * num;
			rc.Right		= this->Right * num;
			rc.Upper		= this->Upper * num;
			rc.Lower		= this->Lower * num;

			return rc;
		}

		rect<T> operator * (const rect<T>& other) const
		{
			rect<T> rc;
			rc.Left			= Left * other.Left;
			rc.Right		= Right * other.Right;
			rc.Upper		= Upper * other.Upper;
			rc.Lower		= Lower * other.Lower;

			return rc;
		}

		// compares size of rectangles
		bool operator<(const rect<T>& other) const
		{
			return getArea() < other.getArea();
		}

		//! Returns size of rectangle
		T getArea() const
		{
			return getWidth() * getHeight();
		}

		//! Returns if a 2d point is within this rectangle.
		//! \return Returns true if the position is within the rectangle, false if not.
		bool isPointInside(T x, T y) const
		{
			return (x >= Left && x <= Right &&
					y >= Upper && y <= Lower);
		}

		//! Returns if the rectangle collides with another rectangle.
		bool isRectCollided(const rect<T>& other) const
		{
			return (Lower > other.Upper &&
					Upper < other.Lower &&
					Right > other.Left &&
					Left < other.Right);
		}

		//! Returns if the rectangle collides with a circle
		bool isRectCollidedWithCircle(const vector2d<T>& point, T radius) const
		{
			vector2d<T> center	= getCenter();
			return	abs(center.X - point.X) < radius + getWidth() / 2 &&
				abs(center.Y - point.Y) < radius + getHeight() / 2;
		}

		bool isCollide_1d(T max0, T min0, T max1, T min1) const
		{
			T tmp;
			if (max0 < min0)
			{
				tmp		= max0;
				max0	= min0;
				min0	= tmp;
			}
			if (max1 < min1)
			{
				tmp		= max1;
				max1	= min1;
				min1	= tmp;
			}

			if (max1 < min0 || min1 > max0)
			{
				return false;
			}

			return true;
		}

		//! Returns if the rectangle collides with a line
		bool isRectCollidedWithLine2d(const line2d<T>& line) const
		{
			// rough test first
			if (isCollide_1d(Right, Left, line.end.X, line.start.X) &&
				isCollide_1d(Lower, Upper, line.end.Y, line.start.Y))
			{
				// accurate test, test line across 2 lines
				line2d<T> l;
				l.start.X			= Left;
				l.start.Y			= Upper;
				l.end.X				= Right;
				l.end.Y				= Lower;

				if (l.isIntersectWithLine2d(line))
				{
					return true;
				}

				l.start.X			= Right;
				l.start.Y			= Upper;
				l.end.X				= Left;
				l.end.Y				= Lower;
				if (l.isIntersectWithLine2d(line))
				{
					return true;
				}
			}
			return  false;
		}

		//! Clips this rectangle with another one.
		void clipAgainst(const rect<T>& other)
		{
			if (other.Right < Right)
				Right = other.Right;
			if (other.Lower < Lower)
				Lower = other.Lower;

			if (other.Left > Left)
				Left = other.Left;
			if (other.Upper > Upper)
				Upper = other.Upper;

			// correct possible invalid rect resulting from clipping
			if (Upper > Lower)
				Upper = Lower;
			if (Left > Right)
				Left = Right;
		}

		//! Moves this rectangle to fit inside another one.
		//! \return: returns true on success, false if not possible
		bool constrainTo(const rect<T>& other)
		{
			if (other.getWidth() < getWidth() || other.getHeight() < getHeight())
				return false;

			T diff = other.Right - Right;
			if (diff < 0)
			{
				Right = Right + diff;
				Left = Left + diff;
			}

			diff = other.Lower - Lower;
			if (diff < 0)
			{
				Lower = Lower + diff;
				Upper = Upper + diff;
			}

			diff = Left - other.Left;
			if (diff < 0)
			{
				Left = Left - diff;
				Right = Right - diff;
			}

			diff = Upper - other.Upper;
			if (diff < 0)
			{
				Upper = Upper - diff;
				Lower = Lower - diff;
			}

			return true;
		}

		//! Returns width of rectangle.
		T getWidth() const
		{
			return Right - Left;
		}

		//! Returns height of rectangle.
		T getHeight() const
		{
			return Lower - Upper;
		}

		//! If the lower right corner of the rect is smaller then the
		//! upper left, the points are swapped.
		void repair()
		{
			if (Right < Left)
			{
				T t = Right;
				Right = Left;
				Left = t;
			}

			if (Lower < Upper)
			{
				T t = Lower;
				Lower = Upper;
				Upper = t;
			}
		}

		//! Returns if the rect is valid to draw. It could be invalid
		//! if the UpperLeftCorner is lower or more right than the
		//! LowerRightCorner, or if any dimension is 0.
		bool isValid() const
		{
			return ((Right >= Left) &&
					(Lower >= Upper));
		}

		//! Returns the center of the rectangle
		vector2d<T> getCenter() const
		{
			return vector2d<T>((Left + Right) / 2,
							   (Upper + Lower) / 2);
		}

		//! Returns the dimensions of the rectangle
		vector2d<T> getSize() const
		{
			return vector2d<T>(getWidth(), getHeight());
		}


		//! Adds a point to the rectangle, causing it to grow bigger,
		//! if point is outside of the box
		//! \param p Point to add into the box.
		void addInternalPoint(const vector2d<T>& p)
		{
			addInternalPoint(p.getX(), p.getY());
		}

		//! Adds a point to the bounding rectangle, causing it to grow bigger,
		//! if point is outside of the box.
		//! \param x X Coordinate of the point to add to this box.
		//! \param y Y Coordinate of the point to add to this box.
		void addInternalPoint(T x, T y)
		{
			if (x > Right)
				Right = x;
			if (y > Lower)
				Lower = y;

			if (x < Left)
				Left = x;
			if (y < Upper)
				Upper = y;
		}

		//! Adds a rectangle to the bounding rectangle, causing it to grow bigger,
		void addInternalRect(const rect<T>& rc)
		{
			addInternalPoint(rc.Left, rc.Upper);
			addInternalPoint(rc.Right, rc.Lower);
		}

		void move(T x, T y)
		{
			Left	+= x;
			Right	+= x;
			Upper	+= y;
			Lower	+= y;
		}

		void move(const vector2d<T>& off)
		{
			Left	+= off.X;
			Right	+= off.X;
			Upper	+= off.Y;
			Lower	+= off.Y;
		}

		void scale(T w, T h)
		{
			Left = Left * w;
			Upper = Upper * h;
			Right = Right * w;
			Lower = Lower * h;
		}

		void scaleFromCenter(float w, float h)
		{
			vector2d<T> center	= getCenter();
			T w_h				= (T)(getWidth() * w / 2);
			T h_h				= (T)(getHeight() * h / 2);
			Left				= center.X - w_h;
			Right				= center.X + w_h;
			Upper				= center.Y - h_h;
			Lower				= center.Y + h_h;
		}


		T Upper, Left;
		T Lower, Right;
	};

	typedef rect<float32> rectf;
	typedef rect<int> recti;
} // end namespace ti

#endif

