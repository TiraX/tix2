/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	class FRenderTargetDx12 : public FRenderTarget
	{
	public:
		FRenderTargetDx12();
		virtual ~FRenderTargetDx12();

		virtual void Destroy() override;
	protected:

	private:
		uint32 RTColorDescriptor[ERTC_COUNT];
		uint32 RTDSDescriptor;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
