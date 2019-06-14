//-*-c++-*-
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#pragma once
#ifndef __IRR_POINT_2D_H_INCLUDED__
#define __IRR_POINT_2D_H_INCLUDED__

namespace tix
{

//! 2d vector template class with lots of operators and methods.
template <typename T>
class vector2d
{
public:

	//!
	typedef T SValueType;

	//! Default constructor (null vector)
	vector2d() : X(0), Y(0) {}
	//! Constructor with two different values
	vector2d(T nx, T ny) : X(nx), Y(ny) {}
	//! Constructor with the same value for both members
	explicit vector2d(T n) : X(n), Y(n) {}
	//! Copy constructor
	vector2d(const vector2d<T>& other) : X(other.getX()), Y(other.getY()) {}

	//! Copy conversion constructor
	template <typename T2>
	explicit vector2d(const vector2d<T2>& other) : X(T(other.getX())), Y(T(other.getY())) {}

	// operators

	T& operator [] (uint32 i)
	{
		TI_ASSERT(i < 2);
		return getDataPtr()[i];
	}

	const T& operator [] (uint32 i) const
	{
		TI_ASSERT(i < 2);
		return getDataPtr()[i];
	}

	vector2d<T> operator-() const { return vector2d<T>(-X, -Y); }

	vector2d<T>& operator=(const vector2d<T>& other) { X = other.getX(); Y = other.getY(); return *this; }

	vector2d<T> operator+(const vector2d<T>& other) const { return vector2d<T>(X + other.getX(), Y + other.getY()); }
	vector2d<T>& operator+=(const vector2d<T>& other) { X+=other.getX(); Y+=other.getY(); return *this; }
	vector2d<T> operator+(const T v) const { return vector2d<T>(X + v, Y + v); }
	vector2d<T>& operator+=(const T v) { X+=v; Y+=v; return *this; }

	vector2d<T> operator-(const vector2d<T>& other) const { return vector2d<T>(X - other.getX(), Y - other.getY()); }
	vector2d<T>& operator-=(const vector2d<T>& other) { X-=other.getX(); Y-=other.getY(); return *this; }
	vector2d<T> operator-(const T v) const { return vector2d<T>(X - v, Y - v); }
	vector2d<T>& operator-=(const T v) { X-=v; Y-=v; return *this; }

	vector2d<T> operator*(const vector2d<T>& other) const { return vector2d<T>(X * other.getX(), Y * other.getY()); }
	vector2d<T>& operator*=(const vector2d<T>& other) { X*=other.getX(); Y*=other.getY(); return *this; }
	vector2d<T> operator*(const T v) const { return vector2d<T>(X * v, Y * v); }
	vector2d<T>& operator*=(const T v) { X*=v; Y*=v; return *this; }

	vector2d<T> operator/(const vector2d<T>& other) const { return vector2d<T>(X / other.getX(), Y / other.getY()); }
	vector2d<T>& operator/=(const vector2d<T>& other) { X/=other.getX(); Y/=other.getY(); return *this; }
	vector2d<T> operator/(const T v) const { return vector2d<T>(X / v, Y / v); }
	vector2d<T>& operator/=(const T v) { X/=v; Y/=v; return *this; }

	bool operator<=(const vector2d<T>&other) const { return X<=other.getX() && Y<=other.getY(); }
	bool operator>=(const vector2d<T>&other) const { return X>=other.getX() && Y>=other.getY(); }

	bool operator<(const vector2d<T>&other) const { return X<other.getX() && Y<other.getY(); }
	bool operator>(const vector2d<T>&other) const { return X>other.getX() && Y>other.getY(); }

	bool operator==(const vector2d<T>& other) const { return other.getX()==X && other.getY()==Y; }
	bool operator!=(const vector2d<T>& other) const { return other.getX()!=X || other.getY()!=Y; }

	// functions

	//! Checks if this vector equals the other one.
	/** Takes floating point rounding errors into account.
	\param other Vector to compare with.
	\return True if the two vector are (almost) equal, else false. */
	bool equals(const vector2d<T>& other) const
	{
		return equals(X, other.getX()) && equals(Y, other.getY());
	}

	vector2d<T>& set(T nx, T ny) {X=nx; Y=ny; return *this; }
	vector2d<T>& set(const vector2d<T>& p) { X=p.getX(); Y=p.getY(); return *this; }

	//! Gets the length of the vector.
	/** \return The length of the vector. */
	T getLength() const { return (T)sqrt((float64)(X*X + Y*Y)); }

	//! Get the squared length of this vector
	/** This is useful because it is much faster than getLength().
	\return The squared length of the vector. */
	T getLengthSQ() const { return X*X + Y*Y; }

	//! Get the dot product of this vector with another.
	/** \param other Other vector to take dot product with.
	\return The dot product of the two vectors. */
	T dotProduct(const vector2d<T>& other) const
	{
		return X*other.getX() + Y*other.getY();
	}

	//! Gets distance from another point.
	/** Here, the vector is interpreted as a point in 2-dimensional space.
	\param other Other vector to measure from.
	\return Distance from other point. */
	T getDistanceFrom(const vector2d<T>& other) const
	{
		return vector2d<T>(X - other.getX(), Y - other.getY()).getLength();
	}

	//! Returns squared distance from another point.
	/** Here, the vector is interpreted as a point in 2-dimensional space.
	\param other Other vector to measure from.
	\return Squared distance from other point. */
	T getDistanceFromSQ(const vector2d<T>& other) const
	{
		return vector2d<T>(X - other.getX(), Y - other.getY()).getLengthSQ();
	}

	//! rotates the point around a center by an amount of degrees.
	/** \param degrees Amount of degrees to rotate by.
	\param center Rotation center.
	\return This vector after transformation. */
	vector2d<T>& rotateBy(float64 degrees, const vector2d<T>& center)
	{
		degrees *= DEGTORAD64;
		const T cs = (T)cos(degrees);
		const T sn = (T)sin(degrees);

		X -= center.getX();
		Y -= center.getY();

		set(X*cs - Y*sn, X*sn + Y*cs);

		X += center.getX();
		Y += center.getY();
		return *this;
	}

	//! Normalize the vector.
	/** The null vector is left untouched.
	\return Reference to this vector, after normalization. */
	vector2d<T>& normalize()
	{
		T l = X*X + Y*Y;
		if (l == 0)
			return *this;
		l = reciprocal_squareroot ( (float32)l );
		X *= l;
		Y *= l;
		return *this;
	}

	//! Calculates the angle of this vector in degrees in the trigonometric sense.
	/** 0 is to the left (9 o'clock), values increase clockwise.
	This method has been suggested by Pr3t3nd3r.
	\return Returns a value between 0 and 360. */
	float64 getAngleTrig() const
	{
		if (X == 0)
			return Y < 0 ? 270 : 90;
		else
		if (Y == 0)
			return X < 0 ? 180 : 0;

		if ( Y > 0)
			if (X > 0)
				return atan(Y/X) * RADTODEG64;
			else
				return 180.0-atan(Y/-X) * RADTODEG64;
		else
			if (X > 0)
				return 360.0-atan(-Y/X) * RADTODEG64;
			else
				return 180.0+atan(-Y/-X) * RADTODEG64;
	}

	//! Calculates the angle of this vector in degrees in the counter trigonometric sense.
	/** 0 is to the right (3 o'clock), values increase counter-clockwise.
	\return Returns a value between 0 and 360. */
	inline float64 getAngle() const
	{
		if (Y == 0) // corrected thanks to a suggestion by Jox
			return X < 0 ? 180 : 0;
		else if (X == 0)
			return Y < 0 ? 90 : 270;

		float64 tmp = Y / getLength();
		tmp = atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG64;

		if (X>0 && Y>0)
			return tmp + 270;
		else
		if (X>0 && Y<0)
			return tmp + 90;
		else
		if (X<0 && Y<0)
			return 90 - tmp;
		else
		if (X<0 && Y>0)
			return 270 - tmp;

		return tmp;
	}

	//! Calculates the angle between this vector and another one in degree.
	/** \param b Other vector to test with.
	\return Returns a value between 0 and 90. */
	inline float64 getAngleWith(const vector2d<T>& b) const
	{
		float64 tmp = X*b.getX() + Y*b.getY();

		if (tmp == 0.0)
			return 90.0;

		tmp = tmp / sqrt((float64)((X*X + Y*Y) * (b.getX()*b.getX() + b.getY()*b.getY())));
		if (tmp < 0.0)
			tmp = -tmp;

		return atan(sqrt(1 - tmp*tmp) / tmp) * RADTODEG64;
	}

	//! Returns if this vector interpreted as a point is on a line between two other points.
	/** It is assumed that the point is on the line.
	\param begin Beginning vector to compare between.
	\param end Ending vector to compare between.
	\return True if this vector is between begin and end, false if not. */
	bool isBetweenPoints(const vector2d<T>& begin, const vector2d<T>& end) const
	{
		if (begin.getX() != end.getX())
		{
			return ((begin.getX() <= X && X <= end.getX()) ||
				(begin.getX() >= X && X >= end.getX()));
		}
		else
		{
			return ((begin.getY() <= Y && Y <= end.getY()) ||
				(begin.getY() >= Y && Y >= end.getY()));
		}
	}

	//! Get the interpolated vector
	/** \param other Other vector to interpolate with.
	\param d Value between 0.0f and 1.0f.
	\return Interpolated vector. */
	vector2d<T> getInterpolated(const vector2d<T>& other, float32 d) const
	{
		T inv = (T) 1.0 - d;
		return vector2d<T>(other.getX()*inv + X*d, other.getY()*inv + Y*d);
	}

	//! Returns (quadratically) interpolated vector between this and the two given ones.
	/** \param v2 Second vector to interpolate with
	\param v3 Third vector to interpolate with
	\param d Value between 0.0f and 1.0f.
	\return Interpolated vector. */
	vector2d<T> getInterpolated_quadratic(const vector2d<T>& v2, const vector2d<T>& v3, const T d) const
	{
		// this*(1-d)*(1-d) + 2 * v2 * (1-d) + v3 * d * d;
		const T inv = (T) 1.0 - d;
		const T mul0 = inv * inv;
		const T mul1 = (T) 2.0 * d * inv;
		const T mul2 = d * d;

		return vector2d<T> ( X * mul0 + v2.getX() * mul1 + v3.getX() * mul2,
					Y * mul0 + v2.getY() * mul1 + v3.getY() * mul2);
	}

	//! Sets this vector to the linearly interpolated vector between a and b.
	/** \param a first vector to interpolate with
	\param b second vector to interpolate with
	\param t value between 0.0f and 1.0f. */
	vector2d<T>& interpolate(const vector2d<T>& a, const vector2d<T>& b, const float32 t)
	{
		X = b.getX() + ( ( a.getX() - b.getX() ) * t );
		Y = b.getY() + ( ( a.getY() - b.getY() ) * t );
		return *this;
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

	void setX(const T& val) { X = val; }
	void setY(const T& val) { Y = val; }


	//! X coordinate of vector.
	T X;
	//! Y coordinate of vector.
	T Y;
};

//! Typedef for float32 2d vector.
typedef vector2d<float32> vector2df;
typedef vector2d<float16> vector2df16;
typedef vector2d<float64> vector2df64;
//! Typedef for integer 2d vector.
typedef vector2d<int32> vector2di;
typedef vector2d<uint16> vector2du16;

template<class S, class T>
vector2d<T> operator*(const S scalar, const vector2d<T>& vector) { return vector*scalar; }

} // end namespace ti

#endif

