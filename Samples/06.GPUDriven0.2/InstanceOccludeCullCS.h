/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FInstanceOccludeCullCS : public FComputeTask
{
public:
	FInstanceOccludeCullCS();
	virtual ~FInstanceOccludeCullCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InOcclusionInfo,
		FUniformBufferPtr InPrimitiveBBoxes,
		FUniformBufferPtr InInstanceMetaInfo,
		FInstanceBufferPtr InInstanceData,
		FGPUCommandSignaturePtr InCommandSignature,
		FGPUCommandBufferPtr InDrawCommandBuffer,
		FUniformBufferPtr InVisibleInstanceIndex,
		FUniformBufferPtr InVisibleInstanceCount,
		FTexturePtr InHiZTexture
		);

	FGPUCommandBufferPtr GetCulledDrawCommandBuffer()
	{
		return CulledDrawCommandBuffer;
	}

	FInstanceBufferPtr GetCompactInstanceBuffer()
	{
		return CompactInstanceData;
	}
private:
	enum
	{
		PARAM_PRIMITIVE_BBOXES,
		PARAM_INSTANCE_METAINFO,
		PARAM_INSTANCE_DATA,
		PARAM_DRAW_COMMAND_BUFFER,
		PARAM_VISIBLE_INSTANCE_INDEX,
		PARAM_VISIBLE_INSTANCE_COUNT,
		PARAM_HIZ_TEXTURE,

		PARAM_COMPACT_INSTANCE_DATA,
		PARAM_CULLED_DRAW_COMMAND_BUFFER,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	FUniformBufferPtr OcclusionInfo;	// b0

	FUniformBufferPtr PrimitiveBBoxes;	// t0
	FUniformBufferPtr InstanceMetaInfo;	// t1
	FInstanceBufferPtr InstanceData;	// t2
	FGPUCommandBufferPtr DrawCommandBuffer;	// t3
	FUniformBufferPtr VisibleInstanceIndex;	// t4
	FUniformBufferPtr VisibleInstanceCount;	 // t5
	FTexturePtr HiZTexture;	// t6

	FInstanceBufferPtr CompactInstanceData;	// u0
	FGPUCommandBufferPtr CulledDrawCommandBuffer;	// u1

	FUniformBufferPtr ResetCommandBuffer;
};
typedef TI_INTRUSIVE_PTR(FInstanceOccludeCullCS) FInstanceOccludeCullCSPtr;
