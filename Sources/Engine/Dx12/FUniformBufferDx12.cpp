/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FUniformBufferDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FUniformBufferDx12::FUniformBufferDx12(uint32 InStructureSizeInBytes, uint32 Elements, uint32 InFlag)
		: FUniformBuffer(InStructureSizeInBytes, Elements, InFlag)
	{
	}

	FUniformBufferDx12::~FUniformBufferDx12()
	{
		TI_ASSERT(IsRenderThread());
	}

	/////////////////////////////////////////////////////////
	FUniformBufferReadableDx12::FUniformBufferReadableDx12(uint32 InStructureSizeInBytes, uint32 Elements, uint32 InFlag)
		: FUniformBufferDx12(InStructureSizeInBytes, Elements, InFlag)
	{
	}

	FUniformBufferReadableDx12::~FUniformBufferReadableDx12()
	{
		TI_ASSERT(IsRenderThread());
	}

	TStreamPtr FUniformBufferReadableDx12::ReadBufferData()
	{
		if (ReadbackResource.GetResource() != nullptr)
		{
			// The code below assumes that the GPU wrote FLOATs to the buffer.
			int32 BufferSize = GetTotalBufferSize();
			if ((GetFlag() & UB_FLAG_COMPUTE_WITH_COUNTER) != 0)
			{
				BufferSize = FRHIDx12::GetUBSizeWithCounter(BufferSize);
			}
			D3D12_RANGE ReadbackBufferRange{ 0, (SIZE_T)BufferSize };
			uint8* Result = nullptr;
			HRESULT Hr = ReadbackResource.GetResource()->Map(0, &ReadbackBufferRange, reinterpret_cast<void**>(&Result));
			TI_ASSERT(SUCCEEDED(Hr));

			TStreamPtr Data = ti_new TStream(BufferSize);
			Data->Put(Result, BufferSize);

			// Code goes here to access the data via pReadbackBufferData.
			D3D12_RANGE EmptyRange{ 0, 0 };
			ReadbackResource.GetResource()->Unmap(0, &EmptyRange);
			return Data;
		}
		return nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12