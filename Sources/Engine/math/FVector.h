/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FVector4
	{
	public:
		FVector4()
			: X(0.f)
			, Y(0.f)
			, Z(0.f)
			, W(1.f)
		{}

		~FVector4()
		{}

		FVector4& operator = (const vector3df& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;

			return *this;
		}

		FVector4& operator = (const SColorf& Other)
		{
			X = Other.R;
			Y = Other.G;
			Z = Other.B;
			W = Other.A;

			return *this;
		}

		FVector4& operator = (const SColor& Other)
		{
			SColorf C(Other);
			X = C.R;
			Y = C.G;
			Z = C.B;
			W = C.A;

			return *this;
		}

		FVector4& operator = (const quaternion& Other)
		{
			X = Other.X;
			Y = Other.Y;
			Z = Other.Z;
			W = Other.W;

			return *this;
		}
	protected:
		float X,Y,Z,W;
	};
}
