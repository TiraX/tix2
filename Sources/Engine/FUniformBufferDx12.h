/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	class FUniformBufferDx12 : public FUniformBuffer
	{
	public:
		FUniformBufferDx12(uint32 InStructSize);
		virtual ~FUniformBufferDx12();
	protected:

	private:
		ComPtr<ID3D12Resource> ConstantBuffer;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
