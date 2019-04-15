/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	template < typename T >
	class FVec4
	{
	public:
		FVec4()
			: X(0.f)
			, Y(0.f)
			, Z(0.f)
			, W(1.f)
		{}

		FVec4(T InX, T InY, T InZ, T InW)
			: X(InX)
			, Y(InY)
			, Z(InZ)
			, W(InW)
		{}

		~FVec4()
		{}

		FVec4& operator = (const vector2d<T>& Other)
		{
			X = Other.X;
			Y = Other.Y;

			return *this;
		}

		FVec4& operator = (const vector3d<T>& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;

			return *this;
		}

		FVec4& operator = (const vector4d<T>& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			W = Other.W;

			return *this;
		}

		FVec4& operator = (const SColorf& Other)
		{
			X = Other.R;
			Y = Other.G;
			Z = Other.B;
			W = Other.A;

			return *this;
		}

		FVec4& operator = (const SColor& Other)
		{
			SColorf C(Other);
			X = C.R;
			Y = C.G;
			Z = C.B;
			W = C.A;

			return *this;
		}

		FVec4& operator = (const quaternion& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			W = Other.W;

			return *this;
		}

		T* getDataPtr()
		{
			return reinterpret_cast<T*>(this);
		}

		const T* getDataPtr() const
		{
			return reinterpret_cast<const float*>(this);
		}
		
		T& operator [] (uint32 i)
		{
			TI_ASSERT(i < 4);
			return getDataPtr()[i];
		}

		const T& operator [] (uint32 i) const
		{
			TI_ASSERT(i < 4);
			return getDataPtr()[i];
		}
	public:
		T X,Y,Z,W;
	};

	typedef FVec4<float> FFloat4;
	typedef FVec4<half> FHalf4;

	class FInt4
	{
	public:
		FInt4()
			: X(0)
			, Y(0)
			, Z(0)
			, W(0)
		{}

		~FInt4()
		{}

		FInt4& operator = (const vector3di& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;

			return *this;
		}

		FInt4& operator = (const SColor& Other)
		{
			X = Other.R;
			Y = Other.G;
			Z = Other.B;
			W = Other.A;

			return *this;
		}

		int32* getDataPtr()
		{
			return reinterpret_cast<int32*>(this);
		}

		const int32* getDataPtr() const
		{
			return reinterpret_cast<const int32*>(this);
		}

		int32& operator [] (uint32 i)
		{
			TI_ASSERT(i < 4);
			return getDataPtr()[i];
		}

		const int32& operator [] (uint32 i) const
		{
			TI_ASSERT(i < 4);
			return getDataPtr()[i];
		}
	public:
		int32 X, Y, Z, W;
	};
}
