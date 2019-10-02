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

		struct FBindingArgument
		{
			uint32 BindingIndex;
			FUniformBufferPtr UniformBuffer;
		};
		TI_API void AddVSPublicArgument(uint32 InBindingIndex, FUniformBufferPtr InUniformBuffer);
		TI_API void AddPSPublicArgument(uint32 InBindingIndex, FUniformBufferPtr InUniformBuffer);

		const TVector<FBindingArgument>& GetVSPublicArguments() const
		{
			return VSPublicArguments;
		}
		const TVector<FBindingArgument>& GetPSPublicArguments() const
		{
			return PSPublicArguments;
		}
	private:

	protected:
		FGPUCommandSignaturePtr GPUCommandSignature;
		TVector<FBindingArgument> VSPublicArguments;
		TVector<FBindingArgument> PSPublicArguments;

	};
} // end namespace tix
