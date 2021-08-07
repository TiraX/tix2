/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FDivergenceCS : public FComputeTask
{
public:
	FDivergenceCS();
	virtual ~FDivergenceCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InParam,
		FTexturePtr InVelocity,
		FTexturePtr InDivergence
		);

private:
	enum
	{
		SRV_VELOCITY,
		UAV_DIVERGENCE,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_Fluid2dParam;

};
typedef TI_INTRUSIVE_PTR(FDivergenceCS) FDivergenceCSPtr;