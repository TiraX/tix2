/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "ComputeUniforms.h"

class FClusterCullCS : public FComputeTask
{
public:
	FClusterCullCS();
	virtual ~FClusterCullCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InViewFrustumUniform,
		FUniformBufferPtr InCollectedCount,
		FUniformBufferPtr InClusterBoundingData,
		FInstanceBufferPtr InInstanceData, 
		FGPUCommandBufferPtr InCommandBuffer,
		FTexturePtr InHiZTexture,
		FUniformBufferPtr InCollectedClusters,
		FUniformBufferPtr InDispatchThreadCount,
		FUniformBufferPtr InCounterResetBuffer
		);

	FUniformBufferPtr GetVisibleClusters()
	{
		return VisibleClusters;
	}

private:
	enum
	{
		SRV_CLUSTER_BOUNDING_DATA,
		SRV_INSTANCE_DATA,
		SRV_DRAW_COMMANDS,
		SRV_HIZ_TEXTURE,
		SRV_COLLECTED_CLUSTERS,

		UAV_VISIBLE_CLUSTERS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Dispatch command signature
	FGPUCommandSignaturePtr DispatchCommandSig;
	FGPUCommandBufferPtr DispatchCommandBuffer;

	// Compute params
	FUniformBufferPtr ViewFrustumUniform;	// b0
	FUniformBufferPtr CollectedCountUniform;	// b1

	FUniformBufferPtr ClusterBoundingData;	// t0
	FInstanceBufferPtr InstanceData;	// t1
	FGPUCommandBufferPtr DrawCommandBuffer;	// t2
	FTexturePtr HiZTexture;	// t3
	FUniformBufferPtr CollectedClusters;	// t4

	FUniformBufferPtr VisibleClusters;	// u0

	FUniformBufferPtr DispatchThreadCount;
	FUniformBufferPtr CounterResetBuffer;

};
typedef TI_INTRUSIVE_PTR(FClusterCullCS) FClusterCullCSPtr;
