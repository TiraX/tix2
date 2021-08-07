/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FMatrix
	{
	public:
		static const int32 NumElements = 16;
		FMatrix()
		{
			memset(M, 0, sizeof(float) * NumElements);
			M[0] = 1.f;
			M[5] = 1.f;
			M[10] = 1.f;
			M[15] = 1.f;
		}
		FMatrix(const matrix4& Mat)
		{
			*this = Mat;
		}
		~FMatrix()
		{}

		FMatrix& operator = (const matrix4& Mat)
		{
#if COMPILE_WITH_RHI_DX12
			// set transposed matrix
			M[0] = Mat[0];
			M[1] = Mat[4];
			M[2] = Mat[8];
			M[3] = Mat[12];

			M[4] = Mat[1];
			M[5] = Mat[5];
			M[6] = Mat[9];
			M[7] = Mat[13];

			M[8] = Mat[2];
			M[9] = Mat[6];
			M[10] = Mat[10];
			M[11] = Mat[14];

			M[12] = Mat[3];
			M[13] = Mat[7];
			M[14] = Mat[11];
			M[15] = Mat[15];
#else
			memcpy(M, Mat.pointer(), sizeof(float) * NumElements);
#endif
			return *this;
		}

		float& operator [] (uint32 Index)
		{
			TI_ASSERT(Index < NumElements);
			return M[Index];
		}

		const float& operator [] (uint32 Index) const
		{
			TI_ASSERT(Index < NumElements);
			return M[Index];
		}

		float* Data()
		{
			return M;
		}

		const float* Data() const
		{
			return M;
		}
	protected:
		float M[NumElements];
	};

	////////////////////////////////////////////////////////////
	class FMatrix3x4
	{
	public:
		static const int32 NumElements = 12;
		FMatrix3x4()
		{
			memset(M, 0, sizeof(float) * NumElements);
			M[0] = 1.f;
			M[5] = 1.f;
			M[10] = 1.f;
		}
		FMatrix3x4(const matrix4& Mat)
		{
			*this = Mat;
		}
		~FMatrix3x4()
		{}

		FMatrix3x4& operator = (const matrix4& Mat)
		{
#if COMPILE_WITH_RHI_DX12
			// set transposed matrix
			M[0] = Mat[0];
			M[1] = Mat[4];
			M[2] = Mat[8];
			M[3] = Mat[12];

			M[4] = Mat[1];
			M[5] = Mat[5];
			M[6] = Mat[9];
			M[7] = Mat[13];

			M[8] = Mat[2];
			M[9] = Mat[6];
			M[10] = Mat[10];
			M[11] = Mat[14];
#else
			memcpy(M, Mat.pointer(), sizeof(float) * NumElements);
#endif
			return *this;
		}

		void SetTranslation(const vector3df& translation)
		{
			M[3] = translation.getX();
			M[7] = translation.getY();
			M[11] = translation.getZ();
		}

		float& operator [] (uint32 Index)
		{
			TI_ASSERT(Index < NumElements);
			return M[Index];
		}

		const float& operator [] (uint32 Index) const
		{
			TI_ASSERT(Index < NumElements);
			return M[Index];
		}

		float* Data()
		{
			return M;
		}

		const float* Data() const
		{
			return M;
		}
	protected:
		float M[NumElements];
	};
}
