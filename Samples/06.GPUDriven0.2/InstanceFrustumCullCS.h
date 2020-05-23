/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FInstanceFrustumCullCS : public FComputeTask
{
public:
	FInstanceFrustumCullCS();
	virtual ~FInstanceFrustumCullCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InFrustumUniform,
		FUniformBufferPtr InPrimitiveBBoxes,
		FUniformBufferPtr InInstanceMetaInfo,
		FInstanceBufferPtr InInstanceData,
		FGPUCommandSignaturePtr InCommandSignature,
		FGPUCommandBufferPtr InDrawCommandBuffer,
		uint32 InTotalClustersCount
	);

	FGPUCommandBufferPtr GetCulledDrawCommandBuffer()
	{
		return CulledDrawCommandBuffer;
	}
	FUniformBufferPtr GetCollectedClusters()
	{
		return CollectedClusters;
	}
	FUniformBufferPtr GetCollectedClustersCount()
	{
		return CollectedClustersCount;
	}
private:
	enum 
	{
		SRV_PRIMITIVE_BBOXES,
		SRV_INSTANCE_METAINFO,
		SRV_INSTANCE_DATA,
		SRV_DRAW_COMMAND_BUFFER,

		UAV_CULLED_DRAW_COMMAND_BUFFER,
		UAV_COLLECTED_CLUSTERS,
		UAV_COLLECTED_CLUSTERS_COUNT,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	FUniformBufferPtr FrustumUniform;	// b0

	FUniformBufferPtr PrimitiveBBoxes;	// t0
	FUniformBufferPtr InstanceMetaInfo;	// t1
	FInstanceBufferPtr InstanceData;	// t2
	FGPUCommandBufferPtr DrawCommandBuffer;	// t3

	FGPUCommandBufferPtr CulledDrawCommandBuffer;	// u0
	FUniformBufferPtr CollectedClusters;	// u1
	FUniformBufferPtr CollectedClustersCount;	// u2

	FUniformBufferPtr ResetCommandBuffer;
};
typedef TI_INTRUSIVE_PTR(FInstanceFrustumCullCS) FInstanceFrustumCullCSPtr;
