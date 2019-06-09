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
		FUniformBufferDx12(uint32 InStructureSizeInBytes, uint32 Elements, uint32 InFlag);
		virtual ~FUniformBufferDx12();

	protected:

	protected:
		FGPUResourceDx12 BufferResource;
		friend class FRHIDx12;
	};

	/////////////////////////////////////////////////////////
	class FUniformBufferReadableDx12 : public FUniformBufferDx12
	{
	public:
		FUniformBufferReadableDx12(uint32 InStructureSizeInBytes, uint32 Elements, uint32 InFlag);
		virtual ~FUniformBufferReadableDx12();

		virtual TStreamPtr ReadBufferData();
	protected:
		FGPUResourceDx12 ReadbackResource;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
