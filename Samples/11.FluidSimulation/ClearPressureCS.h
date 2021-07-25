/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FClearPressureCS : public FComputeTask
{
public:
	FClearPressureCS();
	virtual ~FClearPressureCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InParam,
		FTexturePtr InPressure
		);

private:
	enum
	{
		SRV_PRESSURE,
		UAV_PRESSURE,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_Fluid2dParam;

};
typedef TI_INTRUSIVE_PTR(FClearPressureCS) FClearPressureCSPtr;
