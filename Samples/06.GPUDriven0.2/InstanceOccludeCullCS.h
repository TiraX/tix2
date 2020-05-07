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
		FTexturePtr InHiZTexture,
		FUniformBufferPtr InDispatchThreadCount,
		uint32 InTotalClusterCount
		);

	FGPUCommandBufferPtr GetCulledDrawCommandBuffer()
	{
		return CulledDrawCommandBuffer;
	}

	FInstanceBufferPtr GetCompactInstanceBuffer()
	{
		return CompactInstanceData;
	}

	FUniformBufferPtr GetCollectedClustersCount()
	{
		return CollectedClustersCount;
	}
	FUniformBufferPtr GetCollectedClusters()
	{
		return CollectedClusters;
	}
private:
	enum
	{
		SRV_PRIMITIVE_BBOXES,
		SRV_INSTANCE_METAINFO,
		SRV_INSTANCE_DATA,
		SRV_DRAW_COMMAND_BUFFER,
		SRV_VISIBLE_INSTANCE_INDEX,
		SRV_VISIBLE_INSTANCE_COUNT,
		SRV_HIZ_TEXTURE,

		UAV_COMPACT_INSTANCE_DATA,
		UAV_CULLED_DRAW_COMMAND_BUFFER,
		UAV_COLLECTED_CLUSTERS_COUNT,
		UAV_COLLECTED_CLUSTERS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Dispatch command signature
	FGPUCommandSignaturePtr DispatchCommandSig;
	FGPUCommandBufferPtr DispatchCommandBuffer;

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
	FUniformBufferPtr CollectedClustersCount;	// u2
	FUniformBufferPtr CollectedClusters;	// u3

	FUniformBufferPtr DispatchThreadCount;
	FUniformBufferPtr ResetCommandBuffer;
};
typedef TI_INTRUSIVE_PTR(FInstanceOccludeCullCS) FInstanceOccludeCullCSPtr;
