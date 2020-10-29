//-*-c++-*-
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#pragma once
#ifndef __IRR_POINT_3D_H_INCLUDED__
#define __IRR_POINT_3D_H_INCLUDED__

namespace tix
{

	//! 3d vector template class with lots of operators and methods.
	template < typename T >
	class vector3d
	{
	public:

		//!
		typedef T SValueType;

		//! Default constructor (null vector).
		vector3d() : X(0), Y(0), Z(0) {}
		//! Constructor with three different values
		vector3d(T nx, T ny, T nz) : X(nx), Y(ny), Z(nz) {}
		//! Constructor with the same value for all elements
		explicit vector3d(T n) : X(n), Y(n), Z(n) {}
		//! Copy constructor
		vector3d(const vector3d<T>& other) : X(other.X), Y(other.Y), Z(other.Z) {}
		// operators

		//!
		T& operator [] (uint32 i)
		{
			TI_ASSERT(i < 3);
			return getDataPtr()[i];
		}

		//!
		const T& operator [] (uint32 i) const
		{
			TI_ASSERT(i < 3);
			return getDataPtr()[i];
		}

		vector3d<T> operator-() const { return vector3d<T>(-X, -Y, -Z); }

		vector3d<T>& operator=(const vector3d<T>& other) { X = other.X; Y = other.Y; Z = other.Z; return *this; }

		vector3d<T> operator+(const vector3d<T>& other) const { return vector3d<T>(X + other.X, Y + other.Y, Z + other.Z); }
		vector3d<T>& operator+=(const vector3d<T>& other) { X+=other.X; Y+=other.Y; Z+=other.Z; return *this; }
		vector3d<T> operator+(const T val) const { return vector3d<T>(X + val, Y + val, Z + val); }
		vector3d<T>& operator+=(const T val) { X+=val; Y+=val; Z+=val; return *this; }

		vector3d<T> operator-(const vector3d<T>& other) const { return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z); }
		vector3d<T>& operator-=(const vector3d<T>& other) { X-=other.X; Y-=other.Y; Z-=other.Z; return *this; }
		vector3d<T> operator-(const T val) const { return vector3d<T>(X - val, Y - val, Z - val); }
		vector3d<T>& operator-=(const T val) { X-=val; Y-=val; Z-=val; return *this; }

		vector3d<T> operator*(const vector3d<T>& other) const { return vector3d<T>(X * other.X, Y * other.Y, Z * other.Z); }
		vector3d<T>& operator*=(const vector3d<T>& other) { X*=other.X; Y*=other.Y; Z*=other.Z; return *this; }
		vector3d<T> operator*(const T v) const { return vector3d<T>(X * v, Y * v, Z * v); }
		vector3d<T>& operator*=(const T v) { X*=v; Y*=v; Z*=v; return *this; }

		vector3d<T> operator/(const vector3d<T>& other) const { return vector3d<T>(X / other.X, Y / other.Y, Z / other.Z); }
		vector3d<T>& operator/=(const vector3d<T>& other) { X/=other.X; Y/=other.Y; Z/=other.Z; return *this; }
		vector3d<T> operator/(const T v) const { T i=(T)1.0/v; return vector3d<T>(X * i, Y * i, Z * i); }
		vector3d<T>& operator/=(const T v) { T i=(T)1.0/v; X*=i; Y*=i; Z*=i; return *this; }

		bool operator<=(const vector3d<T>&other) const { return X<=other.X && Y<=other.Y && Z<=other.Z;}
		bool operator>=(const vector3d<T>&other) const { return X>=other.X && Y>=other.Y && Z>=other.Z;}
		bool operator<(const vector3d<T>&other) const { return X<other.X && Y<other.Y && Z<other.Z;}
		bool operator>(const vector3d<T>&other) const { return X>other.X && Y>other.Y && Z>other.Z;}

		bool operator==(const vector3d<T>& other) const
		{
			return X == other.getX() && Y == other.getY() && Z == other.getZ();
		}
		bool operator!=(const vector3d<T>& other) const
		{
			return X != other.getX() || Y != other.getY() || Z != other.getZ();
		}

		// functions

		//! returns if this vector equals the other one, taking f32ing point rounding errors into account
		bool equals(const vector3d<T>& other, const T tolerance = (T)ROUNDING_ERROR_32 ) const
		{
			return ::equals(X, other.X, tolerance) &&
				   ::equals(Y, other.Y, tolerance) &&
				   ::equals(Z, other.Z, tolerance);
		}

		vector3d<T>& set(const T nx, const T ny, const T nz)
		{
			X = nx;
			Y = ny;
			Z = nz;
			return *this;
		}

		vector3d<T>& set(const vector3d<T>& p)
		{
			X = p.X;
			Y = p.Y;
			Z = p.Z;
			return *this;
		}

		//! Get length of the vector.
		T getLength() const { return (T) sqrt((X*X + Y*Y + Z*Z)); }

		//! Get squared length of the vector.
		/** This is useful because it is much faster than getLength().
			\return Squared length of the vector. */
		T getLengthSQ() const { return X*X + Y*Y + Z*Z; }

		//! Get the dot product with another vector.
		T dotProduct(const vector3d<T>& other) const
		{
			return X*other.X + Y*other.Y + Z*other.Z;
		}

		//! Get distance from another point.
		/** Here, the vector is interpreted as point in 3 dimensional space. */
		T getDistanceFrom(const vector3d<T>& other) const
		{
			return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z).getLength();
		}

		//! Returns squared distance from another point.
		/** Here, the vector is interpreted as point in 3 dimensional space. */
		T getDistanceFromSQ(const vector3d<T>& other) const
		{
			return vector3d<T>(X - other.X, Y - other.Y, Z - other.Z).getLengthSQ();
		}

		//! Calculates the cross product with another vector.
		/** \param p Vector to multiply with.
			\return Crossproduct of this vector with p. */
		vector3d<T> crossProduct(const vector3d<T>& p) const
		{
	#if TI_USE_RH
			return vector3d<T>(-Y * p.Z + Z * p.Y, -Z * p.X + X * p.Z, -X * p.Y + Y * p.X);
	#else
			return vector3d<T>(Y * p.Z - Z * p.Y, Z * p.X - X * p.Z, X * p.Y - Y * p.X);
	#endif
		}

		//! Returns if this vector interpreted as a point is on a line between two other points.
		/** It is assumed that the point is on the line.
			\param begin Beginning vector to compare between.
			\param end Ending vector to compare between.
			\return True if this vector is between begin and end, false if not. */
		bool isBetweenPoints(const vector3d<T>& begin, const vector3d<T>& end) const
		{
			const T f = (end - begin).getLengthSQ();
			return getDistanceFromSQ(begin) <= f &&
				getDistanceFromSQ(end) <= f;
		}

		//! Normalizes the vector.
		/** In case of the 0 vector the result is still 0, otherwise
			the length of the vector will be 1.
			TODO: 64 Bit template doesnt work.. need specialized template
			\return Reference to this vector after normalization. */
		vector3d<T>& normalize()
		{
			T l = X*X + Y*Y + Z*Z;
			if (l == 0)
				return *this;
			l = (T) TMath::ReciprocalSquareroot ( (float32)l );
			X *= l;
			Y *= l;
			Z *= l;
			return *this;
		}

		//! Generate a normalized random vector
		void random()
		{
			const int rand_range	= 0x7fff;
			const int rand_range_h	= rand_range / 2;
			X = (T)((rand() & rand_range) - rand_range_h);
			Y = (T)((rand() & rand_range) - rand_range_h);
			Z = (T)((rand() & rand_range) - rand_range_h);
		}

		//! Sets the length of the vector to a newly value
		vector3d<T>& setLength(T newlength)
		{
			normalize();
			return (*this *= newlength);
		}

		//! Inverts the vector.
		vector3d<T>& invert()
		{
			X *= -1.0f;
			Y *= -1.0f;
			Z *= -1.0f;
			return *this;
		}

		//! Rotates the vector by a specified number of degrees around the Y axis and the specified center.
		/** \param degrees Number of degrees to rotate around the Y axis.
			\param center The center of the rotation. */
		void rotateXZBy(float64 degrees, const vector3d<T>& center=vector3d<T>())
		{
			degrees *= DEGTORAD64;
			T cs = (T)cos(degrees);
			T sn = (T)sin(degrees);
			X -= center.X;
			Z -= center.Z;
			set(X*cs - Z*sn, Y, X*sn + Z*cs);
			X += center.X;
			Z += center.Z;
		}

		//! Rotates the vector by a specified number of degrees around the Z axis and the specified center.
		/** \param degrees Number of degrees to rotate around the Z axis.
			\param center The center of the rotation. */
		void rotateXYBy(float64 degrees, const vector3d<T>& center=vector3d<T>())
		{
			degrees *= DEGTORAD64;
			T cs = (T)cos(degrees);
			T sn = (T)sin(degrees);
			X -= center.X;
			Y -= center.Y;
			set(X*cs - Y*sn, X*sn + Y*cs, Z);
			X += center.X;
			Y += center.Y;
		}

		//! Rotates the vector by a specified number of degrees around the X axis and the specified center.
		/** \param degrees Number of degrees to rotate around the X axis.
			\param center The center of the rotation. */
		void rotateYZBy(float64 degrees, const vector3d<T>& center=vector3d<T>())
		{
			degrees *= DEGTORAD64;
			T cs = (T)cos(degrees);
			T sn = (T)sin(degrees);
			Z -= center.Z;
			Y -= center.Y;
			set(X, Y*cs - Z*sn, Y*sn + Z*cs);
			Z += center.Z;
			Y += center.Y;
		}

		//! Returns interpolated vector.
		/** \param other Other vector to interpolate between
			\param d Value between 0.0f and 1.0f. */
		vector3d<T> getInterpolated(const vector3d<T>& other, const T d) const
		{
			return *this + ((other - *this) * d);
		}

		//! Returns interpolated vector. ( quadratic )
		/** \param v2 Second vector to interpolate with
			\param v3 Third vector to interpolate with
			\param d Value between 0.0f and 1.0f. */
		vector3d<T> getInterpolated_quadratic(const vector3d<T>& v2, const vector3d<T>& v3, const T d) const
		{
			// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
			const T inv = (T) 1.0 - d;
			const T mul0 = inv * inv;
			const T mul1 = (T) 2.0 * d * inv;
			const T mul2 = d * d;

			return vector3d<T> ( X * mul0 + v2.X * mul1 + v3.X * mul2,
								 Y * mul0 + v2.Y * mul1 + v3.Y * mul2,
								 Z * mul0 + v2.Z * mul1 + v3.Z * mul2);
		}

		//!
		const T* getDataPtr() const
		{
			return reinterpret_cast<const T*>(this);
		}

		//!
		T* getDataPtr()
		{
			return reinterpret_cast<T*>(this);
		}

		const T& getX() const { return X; }
		const T& getY() const { return Y; }
		const T& getZ() const { return Z; }

		void setX(const T& val) { X = val; }
		void setY(const T& val) { Y = val; }
		void setZ(const T& val) { Z = val; }

		//! X coordinate of the vector
		T X;
		//! Y coordinate of the vector
		T Y;
		//! Z coordinate of the vector
		T Z;
	};

	//! Typedef for a float32 3d vector.
	typedef vector3d<float64> vector3df64;
	typedef vector3d<float32> vector3df;
	typedef vector3d<float16> vector3dh;
	//! Typedef for an integer 3d vector.
	typedef vector3d<int32> vector3di;

	//! Function multiplying a scalar and a vector component-wise.
	template<class S, class T>
	inline
	vector3d<T> operator*(const S scalar, const vector3d<T>& vector)
	{
		return vector*(T)scalar;
	}

} // end namespace tix

namespace std
{
	template <typename T>
	struct hash < tix::vector3d<T> >
	{
		std::size_t operator() (const tix::vector3d<T>& K) const
		{
			return ((hash<T>()(K.X)
				^ (hash<T>()(K.Y) << 1)) >> 1)
				^ (hash<T>()(K.Z) << 1);
		}
	};
}
#endif

