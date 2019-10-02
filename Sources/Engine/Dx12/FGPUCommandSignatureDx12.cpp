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
		sizeof(D3D12_VERTEX_BUFFER_VIEW) + sizeof(D3D12_VERTEX_BUFFER_VIEW) + sizeof(D3D12_INDEX_BUFFER_VIEW), //COMMAND_SET_MESH_BUFFER,
		sizeof(D3D12_DRAW_INDEXED_ARGUMENTS),	//COMMAND_DRAW_INDEXED,
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