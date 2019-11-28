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
		virtual void EncodeEmptyCommand(uint32 CommandIndex) override;
		virtual void EncodeSetVertexBuffer(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			FMeshBufferPtr MeshBuffer
		) override;
		virtual void EncodeSetInstanceBuffer(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			FInstanceBufferPtr InstanceBuffer
		) override;
		virtual void EncodeSetIndexBuffer(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			FMeshBufferPtr MeshBuffer
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
		virtual void EncodeSetDispatch(
			uint32 CommandIndex,
			uint32 ArgumentIndex,
			uint32 ThreadGroupCountX,
			uint32 ThreadGroupCountY,
			uint32 ThreadGroupCountZ
		) override;

		virtual const void* GetCommandData(uint32 CommandIndex) const override;
		virtual void SetCommandData(uint32 CommandIndex, const void* InData, uint32 InDataSize) override;
	private:

	private:
		uint32 CommandsEncoded;
		TStreamPtr CommandBufferData;

		friend class FRHIDx12;
	};
} // end namespace tix
#endif