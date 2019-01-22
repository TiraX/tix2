/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	class FRenderTargetDx12 : public FRenderTarget
	{
	public:
		FRenderTargetDx12(int32 W, int32 H);
		virtual ~FRenderTargetDx12();

	protected:

	private:
		FRenderResourceTablePtr RTColorTable;
		FRenderResourceTablePtr RTDepthTable;

		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
