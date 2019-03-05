/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
#include "FGPUResourceDx12.h"

namespace tix
{
	class FUniformBufferDx12 : public FUniformBuffer
	{
	public:
		FUniformBufferDx12(uint32 InStructureSizeInBytes, uint32 Elements);
		virtual ~FUniformBufferDx12();

		//Temp
		ComPtr<ID3D12Resource> GetConstantBuffer()
		{
			TI_TODO("Remove this temp.");
			return BufferResource.GetResource();
		}
	protected:

	private:
		FGPUResourceDx12 BufferResource;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
