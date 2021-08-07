/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FGradientSubstractCS : public FComputeTask
{
public:
	FGradientSubstractCS();
	virtual ~FGradientSubstractCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InParam,
		FTexturePtr InPressure,
		FTexturePtr InVelocity,
		FTexturePtr OutVelocity
		);

private:
	enum
	{
		SRV_PRESSURE,
		SRV_VELOCITY,
		UAV_VELOCITY,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_Fluid2dParam;

};
typedef TI_INTRUSIVE_PTR(FGradientSubstractCS) FGradientSubstractCSPtr;