/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// Indirect drawing GPU command buffer
	class FGPUCommandBuffer : public FRenderResource
	{
	public:
		FGPUCommandBuffer(FGPUCommandSignaturePtr Signature, uint32 InCommandsCount, uint32 InBufferFlag);
		virtual ~FGPUCommandBuffer();

		FGPUCommandSignaturePtr GetGPUCommandSignature()
		{
			return GPUCommandSignature;
		}

		TI_API virtual uint32 GetEncodedCommandsCount() const = 0;
		TI_API virtual void EncodeEmptyCommand(uint32 CommandIndex) = 0;
		TI_API virtual void EncodeSetVertexBuffer(
			uint32 CommandIndex, 
			uint32 ArgumentIndex,
			FMeshBufferPtr MeshBuffer) = 0;
		TI_API virtual void EncodeSetInstanceBuffer(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			FInstanceBufferPtr InstanceBuffer) = 0;
		TI_API virtual void EncodeSetIndexBuffer(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			FMeshBufferPtr MeshBuffer) = 0;
		TI_API virtual void EncodeSetDrawIndexed(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			uint32 IndexCountPerInstance,
			uint32 InstanceCount,
			uint32 StartIndexLocation,
			uint32 BaseVertexLocation,
			uint32 StartInstanceLocation
		) = 0;
		TI_API virtual void EncodeSetDispatch(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			uint32 ThreadGroupCountX,
			uint32 ThreadGroupCountY,
			uint32 ThreadGroupCountZ
		) = 0;

		// These two method for debug
		TI_API virtual const void* GetCommandData(uint32 CommandIndex) const
		{
			return nullptr;
		}
		TI_API virtual void SetCommandData(uint32 CommandIndex, const void* InData, uint32 InDataSize)
		{}

		FUniformBufferPtr GetCommandBuffer()
		{
			return CommandBuffer;
		}
	private:

	protected:
		FGPUCommandSignaturePtr GPUCommandSignature;

		FUniformBufferPtr CommandBuffer;
	};
} // end namespace tix
