/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include <math.h>	// system math lib

#define F32_AS_int32(f)		(*((int32 *) &(f)))
#define F32_AS_uint32(f)		(*((uint32 *) &(f)))
#define F32_AS_uint32_POINTER(f)	( ((uint32 *) &(f)))

#define F32_VALUE_0		0x00000000
#define F32_VALUE_1		0x3f800000
#define F32_SIGN_BIT		0x80000000U
#define F32_EXPON_MANTISSA	0x7FFFFFFFU

//! code is taken from IceFPU
//! Integer representation of a floating-point value.
#define IR(x)				((uint32&)(x))

//! Absolute integer representation of a floating-point value
#define AIR(x)				(IR(x)&0x7fffffff)

//! Floating-point representation of an integer value.
#define FR(x)				((float32&)(x))

//! integer representation of 1.0
#define IEEE_1_0			0x3f800000
//! integer representation of 255.0
#define IEEE_255_0			0x437f0000

#define	F32_LOWER_0(f)		(F32_AS_uint32(f) >  F32_SIGN_BIT)
#define	F32_LOWER_EQUAL_0(f)	(F32_AS_int32(f) <= F32_VALUE_0)
#define	F32_GREATER_0(f)	(F32_AS_int32(f) >  F32_VALUE_0)
#define	F32_GREATER_EQUAL_0(f)	(F32_AS_uint32(f) <= F32_SIGN_BIT)
#define	F32_EQUAL_1(f)		(F32_AS_uint32(f) == F32_VALUE_1)
#define	F32_EQUAL_0(f)		( (F32_AS_uint32(f) & F32_EXPON_MANTISSA ) == F32_VALUE_0)

// only same sign
#define	F32_A_GREATER_B(a,b)	(F32_AS_int32((a)) > F32_AS_int32((b)))


const float32 ROUNDING_ERROR_32 = 0.00005f;
const float64 ROUNDING_ERROR_64 = 0.000005;

#ifdef PI // make sure we don't collide with a define
#undef PI
#endif
//! Constant for PI.
const float32 PI		= 3.14159265359f;

//! Constant for reciprocal of PI.
const float32 RECIPROCAL_PI	= 1.0f/PI;

//! Constant for half of PI.
const float32 HALF_PI	= PI/2.0f;

#ifdef PI64 // make sure we don't collide with a define
#undef PI64
#endif
//! Constant for 64bit PI.
const float64 PI64		= 3.1415926535897932384626433832795028841971693993751;

//! Constant for 64bit reciprocal of PI.
const float64 RECIPROCAL_PI64 = 1.0/PI64;

//! 32bit Constant for converting from degrees to radians
const float32 DEGTORAD = PI / 180.0f;

//! 32bit constant for converting from radians to degrees (formally known as GRAD_PI)
const float32 RADTODEG   = 180.0f / PI;

//! 64bit constant for converting from degrees to radians (formally known as GRAD_PI2)
const float64 DEGTORAD64 = PI64 / 180.0;

//! 64bit constant for converting from radians to degrees
const float64 RADTODEG64 = 180.0 / PI64;

#define	RAD_TO_DEG(x)					((x)*RADTODEG)
#define	DEG_TO_RAD(x)					((x)*DEGTORAD)

#define TI_INTERPOLATE(src, dest, t)	((dest - src) * (t) + src)
#define ti_max(a, b) ((a) > (b) ? (a) : (b))
#define ti_min(a, b) ((a) < (b) ? (a) : (b))

inline int ti_round(float n)
{
	if (n >= 0.f)
	{
		return (int)(n + 0.5f);
	}
	else
	{
		return (int)(n - 0.5f);
	}
}

inline int ti_abs(int x)
{
	if (x > 0)
		return x;

	return -x;
}

inline float ti_abs(float x)
{
	if (x > 0)
		return x;

	return -x;
}

inline double ti_abs(double x)
{
	if (x > 0)
		return x;

	return -x;
}

inline long ti_abs(long x)
{
	if (x > 0)
		return x;

	return -x;
}

inline int ti_floor(float x)
{
	if (x >= 0.f)
		return (int)x;
	else
	{
		int r	= (int)x;
		if (x == (float)r)
			return r;
		else
			return r - 1;
	}
}

inline int CountBitNum(unsigned int value)
{
	int num = 0;
	while(value)
	{
		value &= (value - 1);
		++ num;
	}
	return num;
}

inline int ti_log(unsigned int value)
{
	int num = 0;
	while (value)
	{
		value >>= 1;
		++ num;
	}
	return num;
}

//! returns a float value between 0.0 ~ 1.0
inline float randomUnit()
{
	const float k_inv	= 1.0f / 0x7fff;
	return (rand() & 0x7fff) * k_inv;
}

//! returns if a equals b, taking possible rounding errors into account
inline bool equals(const float a, const float b, const float tolerance = ROUNDING_ERROR_32)
{
	return (a + tolerance >= b) && (a - tolerance <= b);
}

//! returns if a equals zero, taking rounding errors into account
inline bool iszero(const float32 a, const float32 tolerance = ROUNDING_ERROR_32)
{
	return equals(a, 0.0f, tolerance);
}

//! returns if a equals zero, taking rounding errors into account
inline bool iszero(const int a, const int tolerance = 0)
{
	return ( a & 0x7ffffff ) <= tolerance;
}

inline float reciprocal_squareroot(const float32 x)
{
	// comes from Nvidia
	unsigned int tmp = ((unsigned int)(IEEE_1_0 << 1) + IEEE_1_0 - *(uint32*)&x) >> 1;
	float y = *(float*)&tmp;
	return y * (1.47f - 0.47f * x * y * y);
}

// from Quake3
inline float Q_rsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y   = number;
	i   = * ( long * ) &y;   // evil floating point bit level hacking
	i   = 0x5f3759df - ( i >> 1 ); // what the fuck?
	y   = * ( float * ) &i;
	y   = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
	// y   = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

	return y;
} 

inline uint32 ti_align(uint32 n, uint32 align_num)
{
	return (n + align_num - 1) & (~(align_num - 1));
}

inline int32 ti_align(int32 n, uint32 align_num)
{
	return (n + align_num - 1) & (~(align_num - 1));
}

inline uint32 ti_align4(uint32 n)
{
	return ti_align(n, 4);
}

inline uint32 ti_align16(uint32 n)
{
	return ti_align(n, 16);
}

inline int32 ti_align4(int32 n)
{
	return ti_align(n, 4);
}

inline int32 ti_align16(int32 n)
{
	return ti_align(n, 16);
}

//#include "math/half.hpp"
//using namespace half_float;
//typedef half float16;

#include "math/Vector2d.h"
#include "math/Vector3d.h"
#include "math/Vector4d.h"
#include "math/Line2d.h"
#include "math/Line3d.h"
#include "math/Rect.h"
#include "math/Aabbox3d.h"
#include "math/Plane3d.h"
#include "math/Matrix3.h"
#include "math/Matrix4.h"
#include "math/Quaternion.h"
#include "math/SViewFrustum.h"
#include "math/Triangle3d.h"
#include "math/FMatrix.h"
#include "math/FVector.h"
