/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FAdvectionDyeCS : public FComputeTask
{
public:
	FAdvectionDyeCS();
	virtual ~FAdvectionDyeCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InParam,
		FTexturePtr InVelocity,
		FTexturePtr InDye,
		FTexturePtr OutDye
		);

private:
	enum
	{
		SRV_VELOCITY,
		SRV_DYE_SRC,
		UAV_DYE_DST,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_Fluid2dParam;

};
typedef TI_INTRUSIVE_PTR(FAdvectionDyeCS) FAdvectionDyeCSPtr;
