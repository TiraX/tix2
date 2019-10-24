/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FGPUInstanceFrustumCullCS : public FComputeTask
{
public:
	FGPUInstanceFrustumCullCS();
	virtual ~FGPUInstanceFrustumCullCS();

	void PrepareResources(FRHI * RHI);
	void UpdateComputeArguments(
		FRHI * RHI,
		FUniformBufferPtr PrimitiveBBoxes,
		FUniformBufferPtr InstanceMetaInfo,
		FInstanceBufferPtr SceneInstanceData,
		FUniformBufferPtr InFrustumUniform,
		uint32 InstancesCountIntersectWithFrustum);
	virtual void Run(FRHI * RHI) override;

	FUniformBufferPtr GetVisibleResult()
	{
		return VisibilityResult;
	}
private:

private:
	FRenderResourceTablePtr ResourceTable;
	// Save GPU instance cull result
	FUniformBufferPtr VisibilityResult;
	// Fill Visibility Result buffer default to 1, since some instances from INNER scene tiles do not perform cull, but always visible.
	FUniformBufferPtr FillVisibilityBuffer;

	FUniformBufferPtr FrustumUniform;
	uint32 InstancesNeedToCull;
};
typedef TI_INTRUSIVE_PTR(FGPUInstanceFrustumCullCS) FGPUInstanceFrustumCullCSPtr;
