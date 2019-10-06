/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FGPUTileFrustumCullCS : public FComputeTask
{
public:
	FGPUTileFrustumCullCS();
	virtual ~FGPUTileFrustumCullCS();

	void PrepareResources(FRHI * RHI);
	void UpdateComputeArguments(FRHI * RHI, FScene * Scene, FUniformBufferPtr InFrustumUniform);
	virtual void Run(FRHI * RHI) override;

	FUniformBufferPtr GetVisibilityResult()
	{
		return VisibilityResult;
	}
private:

private:
	FRenderResourceTablePtr ResourceTable;
	FUniformBufferPtr VisibilityResult;

	FUniformBufferPtr FrustumUniform;
};
typedef TI_INTRUSIVE_PTR(FGPUTileFrustumCullCS) FGPUTileFrustumCullCSPtr;
