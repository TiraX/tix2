//-*-c++-*-
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#pragma once
#ifndef __IRR_AABBOX_3D_H_INCLUDED__
#define __IRR_AABBOX_3D_H_INCLUDED__

namespace tix
{

	//! Axis aligned bounding box in 3d dimensional space.
	/** Has some useful methods used with occlusion culling or clipping.
	*/
	template <class T>
	class aabbox3d
	{
	public:
		//! Default Constructor.
		aabbox3d(): MinEdge(T(-1),T(-1),T(-1)), MaxEdge(1,1,1) {}
		//! Constructor with min edge and max edge.
		aabbox3d(const vector3d<T>& min, const vector3d<T>& max): MinEdge(min), MaxEdge(max) {}
		//! Constructor with only one point.
		explicit aabbox3d(const vector3d<T>& init): MinEdge(init), MaxEdge(init) {}
		//! Constructor with min edge and max edge as single values, not vectors.
		aabbox3d(T minx, T miny, T minz, T maxx, T maxy, T maxz): MinEdge(minx, miny, minz), MaxEdge(maxx, maxy, maxz) {}

		// operators
		//! Equality operator
		/** \param other box to compare with.
			\return True if both boxes are equal, else false. */
		inline bool operator==(const aabbox3d<T>& other) const { return (MinEdge == other.MinEdge && other.MaxEdge == MaxEdge);}
		//! Inequality operator
		/** \param other box to compare with.
			\return True if both boxes are different, else false. */
		inline bool operator!=(const aabbox3d<T>& other) const { return !(MinEdge == other.MinEdge && other.MaxEdge == MaxEdge);}
		//! Translate the whole bounding box
		inline aabbox3d<T>& operator+=(const vector3d<T>& other) 
		{ 
			MinEdge += other;
			MaxEdge += other;
			return *this;
		}
		

		// functions

		//! Adds a point to the bounding box
		/** The box grows bigger, if point was outside of the box.
			\param p: Point to add into the box. */
		void addInternalPoint(const vector3d<T>& p)
		{
			addInternalPoint(p.getX(), p.getY(), p.getZ());
		}

		//! Adds another bounding box
		/** The box grows bigger, if the newly box was outside of the box.
			\param b: Other bounding box to add into this box. */
		void addInternalBox(const aabbox3d<T>& b)
		{
			addInternalPoint(b.MaxEdge);
			addInternalPoint(b.MinEdge);
		}

		//! Resets the bounding box to a one-point box.
		/** \param x X coord of the point.
			\param y Y coord of the point.
			\param z Z coord of the point. */
		void reset(T x, T y, T z)
		{
			MaxEdge.set(x,y,z);
			MinEdge = MaxEdge;
		}

		//! Resets the bounding box.
		/** \param initValue New box to set this one to. */
		void reset(const aabbox3d<T>& initValue)
		{
			*this = initValue;
		}

		//! Resets the bounding box to a one-point box.
		/** \param initValue New point. */
		void reset(const vector3d<T>& initValue)
		{
			MaxEdge = initValue;
			MinEdge = initValue;
		}
	#undef max
	#undef min
		//! Invalidate bounding box
		void invalidate()
		{
			MaxEdge = vector3d<T>(std::numeric_limits<T>::min(),
								  std::numeric_limits<T>::min(),
								  std::numeric_limits<T>::min());
			MinEdge = vector3d<T>(std::numeric_limits<T>::max(),
								  std::numeric_limits<T>::max(),
								  std::numeric_limits<T>::max());
		}

		//! Adds a point to the bounding box
		/** The box grows bigger, if point is outside of the box.
			\param x X coordinate of the point to add to this box.
			\param y Y coordinate of the point to add to this box.
			\param z Z coordinate of the point to add to this box. */
		void addInternalPoint(T x, T y, T z)
		{
			if (x>MaxEdge.getX()) MaxEdge.setX(x);
			if (y>MaxEdge.getY()) MaxEdge.setY(y);
			if (z>MaxEdge.getZ()) MaxEdge.setZ(z);

			if (x<MinEdge.getX()) MinEdge.setX(x);
			if (y<MinEdge.getY()) MinEdge.setY(y);
			if (z<MinEdge.getZ()) MinEdge.setZ(z);
		}

		//! Determines if a point is within this box.
		/** \param p: Point to check.
			\return True if the point is within the box and false if not */
		bool isPointInside(const vector3d<T>& p) const
		{
			return (p.getX() >= MinEdge.getX() && p.getX() <= MaxEdge.getX() &&
					p.getY() >= MinEdge.getY() && p.getY() <= MaxEdge.getY() &&
					p.getZ() >= MinEdge.getZ() && p.getZ() <= MaxEdge.getZ());
		}

		//! Determines if a point is within this box and its borders.
		/** \param p: Point to check.
			\return True if the point is within the box and false if not. */
		bool isPointTotalInside(const vector3d<T>& p) const
		{
			return (p.getX() > MinEdge.getX() && p.getX() < MaxEdge.getX() &&
					p.getY() > MinEdge.getY() && p.getY() < MaxEdge.getY() &&
					p.getZ() > MinEdge.getZ() && p.getZ() < MaxEdge.getZ());
		}

		//! Determines if the box intersects with another box.
		/** \param other: Other box to check a intersection with.
			\return True if there is an intersection with the other box,
			otherwise false. */
		bool intersectsWithBox(const aabbox3d<T>& other) const
		{
			return (MinEdge <= other.MaxEdge && MaxEdge >= other.MinEdge);
		}

		//! Check if this box is completely inside the 'other' box.
		/** \param other: Other box to check against.
			\return True if this box is completly inside the other box,
			otherwise false. */
		bool isFullInside(const aabbox3d<T>& other) const
		{
			return MinEdge >= other.MinEdge && MaxEdge <= other.MaxEdge;
		}

		//! Tests if the box intersects with a point
		/** \param point: Point to test intersection with.
			\return True if there is an intersection , else false. */
		bool intersectsWithPoint(const vector3d<T>& point) const
		{
			bool r;
			r =  (MinEdge.getX() <= point.getX() && point.getX() <= MaxEdge.getX());
			r &= (MinEdge.getY() <= point.getY() && point.getY() <= MaxEdge.getY());
			r &= (MinEdge.getZ() <= point.getZ() && point.getZ() <= MaxEdge.getZ());
			return r;
		}

		//! Tests if the box intersects with a line
		/** \param linemiddle Center of the line.
			\param linevect Vector of the line.
			\param halflength Half length of the line.
			\return True if there is an intersection, else false. */
		bool intersectsWithLine_impl_irr(const vector3d<T>& linemiddle,
										 const vector3d<T>& linevect,
										 T halflength) const
		{
			const vector3d<T> e = getExtent() * (T)0.5;
			const vector3d<T> t = getCenter() - linemiddle;

			if ((fabs(t.getX()) > e.getX() + halflength * fabs(linevect.getX())) ||
				(fabs(t.getY()) > e.getY() + halflength * fabs(linevect.getY())) ||
				(fabs(t.getZ()) > e.getZ() + halflength * fabs(linevect.getZ())) )
				return false;

			T r = e.getY() * (T)fabs(linevect.getZ()) + e.getZ() * (T)fabs(linevect.getY());
			if (fabs(t.getY()*linevect.getZ() - t.getZ()*linevect.getY()) > r )
				return false;

			r = e.getX() * (T)fabs(linevect.getZ()) + e.getZ() * (T)fabs(linevect.getX());
			if (fabs(t.getZ()*linevect.getX() - t.getX()*linevect.getZ()) > r )
				return false;

			r = e.getX() * (T)fabs(linevect.getY()) + e.getY() * (T)fabs(linevect.getX());
			if (fabs(t.getX()*linevect.getY() - t.getY()*linevect.getX()) > r)
				return false;

			return true;
		}

		//! Tests if the box intersects with a line segment in 1D
		/** \param line: Line to test intersection with.
			\return True if there is an intersection , else false. */
		bool intersectsWithLine_impl_1d (T  bmin,		// min value of the bounding box
										 T  bmax,		// max value of the bounding box
										 T  si,		    // start of the line segment
										 T  ei,         // end of the line segment
										 T& fst,        // given start value to compare (start with 0)
										 T& fet         // given end value to compare (start with 1)
										) const
		{
			// The algorithm need to know which of the start or the end of the 
			// segment is smaller; the variable could be swapped, but it's faster
			// to duplicate the code.
			T   st, et;
			T   di = ei - si;  

			if (si < ei) 
			{  
				if (si > bmax || ei < bmin) 
					return false;   // outside AABB
				st = (si < bmin) ? (bmin - si) / di: 0; // cut / inclusion 
				et = (ei > bmax) ? (bmax - si) / di: 1; // cut / inclusion 
			} 
			else 
			{  
				if (ei > bmax || si < bmin)  
					return false;   //  outside AABB
				st = (si > bmax)? (bmax - si) / di: 0;  // cut / inclusion  
				et = (ei < bmin)? (bmin - si) / di: 1;  // cut / inclusion
			}

			if (st > fst)   // Compare with prev results - the furthest the start, the better
				fst = st;  
			if (et < fet)   // Compare with prev results - the closest the end, the better
				fet = et;

			if (fet < fst)  // result turned to be outside.
				return false;   

			return true;    // collision exist
		}

	
		//! Tests if the box intersects with a line
		/** \param line: Line to test intersection with.
			\return True if there is an intersection , else false. */
		bool intersectsWithLine(const line3d<T>& line) const
		{
			T   fst, fet;
			return intersectsWithSegment( line, fst, fet );
		}

		//! Tests if the box intersects with a segment
		/** \param line: Line to test intersection with.
			\return True if there is an intersection , else false. 
			Also returns two values which are the relative distance to the collision with the 
			nearest plane (steping in) and the relative distance to the far plane (steping out).
			*/
		bool intersectsWithSegment( const line3d<T>& line, T& fst, T& fet ) const
		{
			fst = 0;
			fet = 1;
			return intersectsWithLine_impl_1d (MinEdge.getX(), MaxEdge.getX(), line.start.getX(), line.end.getX(), fst, fet)
				&& intersectsWithLine_impl_1d (MinEdge.getY(), MaxEdge.getY(), line.start.getY(), line.end.getY(), fst, fet)
				&& intersectsWithLine_impl_1d (MinEdge.getZ(), MaxEdge.getZ(), line.start.getZ(), line.end.getZ(), fst, fet);
		}

		//! Get center of the bounding box
		/** \return Center of the bounding box. */
		vector3d<T> getCenter() const
		{
			return (MinEdge + MaxEdge) / 2;
		}


		//! Get extent of the box
		/** \return Extent of the bounding box. */
		vector3d<T> getExtent() const
		{
			return MaxEdge - MinEdge;
		}

		void extend(float ratio)
		{
			vector3d<T> center	= getCenter();
			vector3d<T> extent	= getExtent();
			extent				*= ratio * 0.5f;
			MaxEdge				= center + extent;
			MinEdge				= center - extent;
		}

		void extend(float x, float y, float z)
		{
			vector3d<T> center	= getCenter();
			vector3d<T> extent	= getExtent();
			extent				*= vector3df(x, y, z) * 0.5f;
			MaxEdge				= center + extent;
			MinEdge				= center - extent;
		}


		//! Stores all 8 edges of the box into an array
		/** \param edges: Pointer to array of 8 edges. */
		void getEdges(vector3d<T> *edges) const
		{
			const vector3d<T> middle = getCenter();
			const vector3d<T> diag = middle - MaxEdge;

			/*
			  Edges are stored in this way:
			  Hey, am I an ascii artist, or what? :) niko.
				 3---------/7
				/|        / |
			   / |       /  |
			  1---------5   |
			  |  2- - - |- -6
			  | /       |  /
			  |/        | /
			  0---------4/
			*/

			edges[0].set(middle.getX() + diag.getX(), middle.getY() + diag.getY(), middle.getZ() + diag.getZ());
			edges[1].set(middle.getX() + diag.getX(), middle.getY() - diag.getY(), middle.getZ() + diag.getZ());
			edges[2].set(middle.getX() + diag.getX(), middle.getY() + diag.getY(), middle.getZ() - diag.getZ());
			edges[3].set(middle.getX() + diag.getX(), middle.getY() - diag.getY(), middle.getZ() - diag.getZ());
			edges[4].set(middle.getX() - diag.getX(), middle.getY() + diag.getY(), middle.getZ() + diag.getZ());
			edges[5].set(middle.getX() - diag.getX(), middle.getY() - diag.getY(), middle.getZ() + diag.getZ());
			edges[6].set(middle.getX() - diag.getX(), middle.getY() + diag.getY(), middle.getZ() - diag.getZ());
			edges[7].set(middle.getX() - diag.getX(), middle.getY() - diag.getY(), middle.getZ() - diag.getZ());
		}


		//! Check if the box is empty.
		/** This means that there is no space between the min and max
			edge.
			\return True if box is empty, else false. */
		bool isEmpty() const
		{
			return MinEdge.equals ( MaxEdge );
		}


		//! Repairs the box.
		/** Necessary if for example MinEdge and MaxEdge are swapped. */
		void repair()
		{
			T t;

			if (MinEdge.getX() > MaxEdge.getX())
			{ t=MinEdge.getX(); MinEdge.setX(MaxEdge.getX()); MaxEdge.setX(t); }
			if (MinEdge.getY() > MaxEdge.getY())
			{ t=MinEdge.getY(); MinEdge.setY(MaxEdge.getY()); MaxEdge.setY(t); }
			if (MinEdge.getZ() > MaxEdge.getZ())
			{ t=MinEdge.getZ(); MinEdge.setZ(MaxEdge.getZ()); MaxEdge.setZ(t); }
		}

		//! Calculates a newly interpolated bounding box.
		/** \param other: other box to interpolate between
			\param d: value between 0.0f and 1.0f.
			\return Interpolated box. */
		aabbox3d<T> getInterpolated(const aabbox3d<T>& other, float32 d) const
		{
			float32 inv = 1.0f - d;
			return aabbox3d<T>((other.MinEdge*inv) + (MinEdge*d),
							   (other.MaxEdge*inv) + (MaxEdge*d));
		}

		//! Get the volume enclosed by the box in cubed units
		T getVolume() const
		{
			const vector3d<T> e = getExtent();
			return e.getX() * e.getY() * e.getZ();
		}

		//! Get the surface area of the box in squared units
		T getArea() const
		{
			const vector3d<T> e = getExtent();
			return 2*(e.getX()*e.getY() + e.getX()*e.getZ() + e.getY()*e.getZ());
		}

		void move(const vector3d<T>& pos)
		{
			MinEdge		+= pos;
			MaxEdge		+= pos;
		}

		//! The near edge
		vector3d<T> MinEdge;
		//! The far edge
		vector3d<T> MaxEdge;
	};

	//! Typedef for a f32 3d bounding box.
	typedef aabbox3d<float> aabbox3df;
	//! Typedef for an integer 3d bounding box.
	typedef aabbox3d<int> aabbox3di;

} // end namespace ti

#endif

