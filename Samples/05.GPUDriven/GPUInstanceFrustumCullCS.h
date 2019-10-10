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
		FUniformBufferPtr InTileVisbleInfo,
		FUniformBufferPtr PrimitiveBBoxes,
		FUniformBufferPtr InstanceMetaInfo,
		FInstanceBufferPtr SceneInstanceData,
		FUniformBufferPtr InFrustumUniform);
	virtual void Run(FRHI * RHI) override;

private:

private:
	FRenderResourceTablePtr ResourceTable;
	FUniformBufferPtr VisibilityResult;

	FUniformBufferPtr FrustumUniform;
};
typedef TI_INTRUSIVE_PTR(FGPUInstanceFrustumCullCS) FGPUInstanceFrustumCullCSPtr;
