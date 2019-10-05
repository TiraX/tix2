/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
#include "FGPUResourceDx12.h"

namespace tix
{
	// Indirect drawing GPU command buffer
	class FGPUCommandBufferDx12 : public FGPUCommandBuffer
	{
	public:
		FGPUCommandBufferDx12(FGPUCommandSignaturePtr Signature, uint32 InCommandsCount, uint32 InBufferFlag);
		virtual ~FGPUCommandBufferDx12();

		virtual uint32 GetEncodedCommandsCount() const override;
		virtual void EncodeSetMeshBuffer(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			FMeshBufferPtr MeshBuffer, 
			FInstanceBufferPtr InstanceBuffer
		) override;
		virtual void EncodeSetDrawIndexed(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			uint32 IndexCountPerInstance,
			uint32 InstanceCount,
			uint32 StartIndexLocation,
			uint32 BaseVertexLocation,
			uint32 StartInstanceLocation
		) override;
	private:

	private:
		uint32 CommandsEncoded;
		TStreamPtr CommandBufferData;

		friend class FRHIDx12;
	};
} // end namespace tix
#endif