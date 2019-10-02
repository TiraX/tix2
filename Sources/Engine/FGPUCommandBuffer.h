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
		FGPUCommandBuffer(FGPUCommandSignaturePtr Signature, uint32 InCommandsCount);
		virtual ~FGPUCommandBuffer();

		FGPUCommandSignaturePtr GetGPUCommandSignature()
		{
			return GPUCommandSignature;
		}

		TI_API virtual uint32 GetEncodedCommandsCount() const = 0;
		TI_API virtual void EncodeSetMeshBuffer(
			uint32 CommandIndex, 
			uint32 ArgumentIndex,
			FMeshBufferPtr MeshBuffer, 
			FInstanceBufferPtr InstanceBuffer) = 0;
		TI_API virtual void EncodeSetDrawIndexed(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			uint32 IndexCountPerInstance,
			uint32 InstanceCount,
			uint32 StartIndexLocation,
			uint32 BaseVertexLocation,
			uint32 StartInstanceLocation
		) = 0;
	private:

	protected:
		FGPUCommandSignaturePtr GPUCommandSignature;
	};
} // end namespace tix
