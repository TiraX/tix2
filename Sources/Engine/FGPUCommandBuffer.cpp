/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FGPUCommandBuffer.h"

namespace tix
{
	FGPUCommandBuffer::FGPUCommandBuffer(FGPUCommandSignaturePtr Signature, uint32 InCommandsCount)
		: FRenderResource(RRT_GPU_COMMAND_BUFFER)
		, GPUCommandSignature(Signature)
	{
	}

	FGPUCommandBuffer::~FGPUCommandBuffer()
	{
	}

	void FGPUCommandBuffer::AddVSPublicArgument(uint32 InBindingIndex, FUniformBufferPtr InUniformBuffer)
	{
		FBindingArgument BindingArgument;
		BindingArgument.BindingIndex = InBindingIndex;
		BindingArgument.UniformBuffer = InUniformBuffer;
		VSPublicArguments.push_back(BindingArgument);
	}

	void FGPUCommandBuffer::AddPSPublicArgument(uint32 InBindingIndex, FUniformBufferPtr InUniformBuffer)
	{
		FBindingArgument BindingArgument;
		BindingArgument.BindingIndex = InBindingIndex;
		BindingArgument.UniformBuffer = InUniformBuffer;
		PSPublicArguments.push_back(BindingArgument);
	}
}
