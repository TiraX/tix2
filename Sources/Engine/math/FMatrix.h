/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// FMeshBuffer, hold vertex buffer and index buffer render resource
	class FMatrix
	{
	public:
		FMatrix()
		{
			memset(M, 0, sizeof(float) * 16);
			M[0] = 1.f;
			M[5] = 1.f;
			M[10] = 1.f;
			M[15] = 1.f;
		}
		FMatrix(const matrix4& mat)
		{
			memcpy(M, mat.pointer(), sizeof(float) * 16);
		}
		~FMatrix()
		{}

		FMatrix& operator = (const matrix4& mat)
		{
			memcpy(M, mat.pointer(), sizeof(float) * 16);
			return *this;
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
		float M[16];
	};
}
