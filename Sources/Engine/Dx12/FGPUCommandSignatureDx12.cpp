/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FGPUCommandSignatureDx12.h"

#if COMPILE_WITH_RHI_DX12
namespace tix
{
	const uint32 FGPUCommandSignatureDx12::GPU_COMMAND_STRIDE[GPU_COMMAND_TYPE_COUNT] =
	{
		sizeof(D3D12_VERTEX_BUFFER_VIEW),		//GPU_COMMAND_SET_VERTEX_BUFFER
		sizeof(D3D12_VERTEX_BUFFER_VIEW),		//GPU_COMMAND_SET_INSTANCE_BUFFER
		sizeof(D3D12_INDEX_BUFFER_VIEW),		//GPU_COMMAND_SET_INDEX_BUFFER,
		sizeof(D3D12_DRAW_INDEXED_ARGUMENTS),	//GPU_COMMAND_DRAW_INDEXED,
		sizeof(D3D12_DISPATCH_ARGUMENTS),		//GPU_COMMAND_DISPATCH
		sizeof(uint32),							//GPU_COMMAND_CONSTANT
		sizeof(uint64),							//GPU_COMMAND_CONSTANT_BUFFER
		sizeof(uint64),							//GPU_COMMAND_SHADER_RESOURCE
		sizeof(uint64)							//GPU_COMMAND_UNORDERED_ACCESS
	};

	FGPUCommandSignatureDx12::FGPUCommandSignatureDx12(FPipelinePtr InPipeline, const TVector<E_GPU_COMMAND_TYPE>& CommandStructure)
		: FGPUCommandSignature(InPipeline, CommandStructure)
		, CommandStrideInBytes(0)
	{
	}

	FGPUCommandSignatureDx12::~FGPUCommandSignatureDx12()
	{
		CommandSignature = nullptr;
	}
}
#endif