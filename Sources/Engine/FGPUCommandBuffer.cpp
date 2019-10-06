/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FGPUCommandBuffer.h"

namespace tix
{
	FGPUCommandBuffer::FGPUCommandBuffer(FGPUCommandSignaturePtr Signature, uint32 InCommandsCount, uint32 InBufferFlag)
		: FRenderResource(RRT_GPU_COMMAND_BUFFER)
		, GPUCommandSignature(Signature)
	{
		VSPublicArguments.resize(MAX_BINDING_UNIFORMS);
		PSPublicArguments.resize(MAX_BINDING_UNIFORMS);
	}

	FGPUCommandBuffer::~FGPUCommandBuffer()
	{
	}

	void FGPUCommandBuffer::AddVSPublicArgument(uint32 InBindingIndex, FUniformBufferPtr InUniformBuffer)
	{
		VSPublicArguments[InBindingIndex] = InUniformBuffer;
	}

	void FGPUCommandBuffer::AddPSPublicArgument(uint32 InBindingIndex, FUniformBufferPtr InUniformBuffer)
	{
		PSPublicArguments[InBindingIndex] = InUniformBuffer;
	}
}
