/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

class FMeanIllumLutCS : public FComputeTask
{
public:
	static const int32 LUT_W = 32;
	static const int32 LUT_H = 32;

	FMeanIllumLutCS();
	virtual ~FMeanIllumLutCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FUniformBufferPtr InAtmosphereParam,
		FTexturePtr InTransmittanceLut
	);

	FTexturePtr GetMultiScatteredLuminanceLut()
	{
		return MultiScatteredLuminanceLut;
	}
private:
	enum
	{
		SRV_LUT_TRANSMITTANCE,
		UAV_LUT_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	FUniformBufferPtr AtmosphereParam;
	FTexturePtr TransmittanceLut;
	FTexturePtr MultiScatteredLuminanceLut;
};
typedef TI_INTRUSIVE_PTR(FMeanIllumLutCS) FMeanIllumLutCSPtr;
