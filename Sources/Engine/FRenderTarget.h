/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderTarget : public FRenderResource
	{
	public:
		FRenderTarget(E_RESOURCE_FAMILY InFamily);
		virtual ~FRenderTarget();

		void SetValidColorBufferCount(int32 N)
		{
			ColorBuffers = N;
		}

		int32 GetColorBufferCount()
		{
			return ColorBuffers;
		}
	protected:

	protected:
		int32 ColorBuffers;
	};
}
