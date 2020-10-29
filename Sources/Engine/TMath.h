/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include <math.h>	// system math lib
#include <cmath>

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


namespace tix
{
	class TMath
	{
	public:
		static inline int32 Round(float n)
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

		template< class T >
		static inline T DegToRad(const T Degree)
		{
			return Degree * DEGTORAD;
		}

		template< class T >
		static inline T RadToDeg(const T Radian)
		{
			return Radian * RADTODEG;
		}

		template< class T >
		static inline T Lerp(const T src, const T dest, const float t)
		{
			return (T)((dest - src) * t + src);
		}

		template< class T >
		static inline T Max(const T a, const T b)
		{
			return a > b ? a : b;
		}

		template< class T >
		static inline T Min(const T a, const T b)
		{
			return a < b ? a : b;
		}

		template< class T >
		static inline T Max3(const T a, const T b, const T c)
		{
			return Max(Max(a, b), c);
		}

		template< class T >
		static inline T Min3(const T a, const T b, const T c)
		{
			return Min(Min(a, b), c);
		}

		template< class T >
		static inline T Clamp(const T X, const T Min, const T Max)
		{
			return X < Min ? Min : X < Max ? X : Max;
		}

		template< class T >
		static inline T Abs(const T x)
		{
			if (x > 0)
				return x;

			return -x;
		}

		static inline int32 Floor(float x)
		{
			if (x >= 0.f)
				return (int32)x;
			else
			{
				int32 r = (int32)x;
				if (x == (float)r)
					return r;
				else
					return r - 1;
			}
		}

		static inline int32 CountBitNum(uint32 value)
		{
			int32 num = 0;
			while (value)
			{
				value &= (value - 1);
				++num;
			}
			return num;
		}

		static inline int32 Log(uint32 value)
		{
			int num = 0;
			while (value)
			{
				value >>= 1;
				++num;
			}
			return num;
		}

		static inline void RandSeed(uint32 Seed)
		{
			srand(Seed);
		}

		//! returns a float value between 0.0 ~ 1.0
		static inline float RandomUnit()
		{
			const float k_inv = 1.0f / RAND_MAX;
			return (rand() & 0x7fff) * k_inv;
		}

		//! returns if a equals b, taking possible rounding errors into account
		static inline bool Equals(const float a, const float b, const float tolerance = ROUNDING_ERROR_32)
		{
			return (a + tolerance >= b) && (a - tolerance <= b);
		}

		//! returns if a equals zero, taking rounding errors into account
		static inline bool IsZero(const float32 a, const float32 tolerance = ROUNDING_ERROR_32)
		{
			return Equals(a, 0.0f, tolerance);
		}

		static inline float ReciprocalSquareroot(const float32 x)
		{
			// comes from Nvidia
			uint32 tmp = ((unsigned int)(IEEE_1_0 << 1) + IEEE_1_0 - *(uint32*)&x) >> 1;
			float y = *(float*)&tmp;
			return y * (1.47f - 0.47f * x * y * y);
		}

		// from Quake3
		static inline float Q_rsqrt(float number)
		{
			long i;
			float x2, y;
			const float threehalfs = 1.5F;

			x2 = number * 0.5F;
			y = number;
			i = *(long*)&y;   // evil floating point bit level hacking
			i = 0x5f3759df - (i >> 1); // what the fuck?
			y = *(float*)&i;
			y = y * (threehalfs - (x2 * y * y)); // 1st iteration
			// y   = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration, this can be removed

			return y;
		}

		template<class T>
		static inline T Align(T n, uint32 align_num)
		{
			return (n + align_num - 1) & (~(align_num - 1));
		}

		template<class T>
		static inline T Align4(T n)
		{
			return Align<T>(n, 4);
		}

		template<class T>
		static inline T Align16(T n)
		{
			return Align<T>(n, 16);
		}

		// From UE4 
		/**
		 * Computes the base 2 logarithm for an integer value that is greater than 0.
		 * The result is rounded down to the nearest integer.
		 *
		 * @param Value		The value to compute the log of
		 * @return			Log2 of Value. 0 if Value is 0.
		 */
#if defined (TI_PLATFORM_WIN32)
#pragma intrinsic( _BitScanReverse )
#endif
		static inline uint32 FloorLog2(uint32 Value)
		{
#if defined (TI_PLATFORM_WIN32)
			// Use BSR to return the log2 of the integer
			unsigned long Log2;
			if (_BitScanReverse(&Log2, Value) != 0)
			{
				return Log2;
			}

			return 0;
#else
			// see http://codinggorilla.domemtech.com/?p=81 or http://en.wikipedia.org/wiki/Binary_logarithm but modified to return 0 for a input value of 0
			// 686ms on test data
			uint32 pos = 0;
			if (Value >= 1 << 16) { Value >>= 16; pos += 16; }
			if (Value >= 1 << 8) { Value >>= 8; pos += 8; }
			if (Value >= 1 << 4) { Value >>= 4; pos += 4; }
			if (Value >= 1 << 2) { Value >>= 2; pos += 2; }
			if (Value >= 1 << 1) { pos += 1; }
			return (Value == 0) ? 0 : pos;
#endif

		}
	};
}



#include "Math/Vector2d.h"
#include "Math/Vector3d.h"
#include "Math/Vector4d.h"
#include "Math/Line2d.h"
#include "Math/Line3d.h"
#include "Math/Rect.h"
#include "Math/Aabbox3d.h"
#include "Math/Plane3d.h"
#include "Math/Matrix3.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
#include "Math/SViewFrustum.h"
#include "Math/Triangle3d.h"
#include "Math/FMatrix.h"
#include "Math/FVector.h"
