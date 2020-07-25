/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

class FDistantSkyLightLutCS : public FComputeTask
{
public:
	static const int32 LUT_W = 1;
	static const int32 LUT_H = 1;

	FDistantSkyLightLutCS();
	virtual ~FDistantSkyLightLutCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FUniformBufferPtr InAtmosphereParam,
		FTexturePtr InTransmittanceLut,
		FTexturePtr InMultiScatteredLuminanceLut
	);

private:
	enum
	{
		SRV_LUT_TRANSMITTANCE,
		SRV_LUT_MULTI_SCATTERED_LUMINANCE,
		SRV_SPHERE_SAMPLES,
		UAV_LUT_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	FUniformBufferPtr AtmosphereParam;
	FTexturePtr TransmittanceLut;
	FTexturePtr MultiScatteredLuminanceLut;
	FUniformBufferPtr UniformSphereSamplesBuffer;

	FTexturePtr DistantSkyLightLut;
};
typedef TI_INTRUSIVE_PTR(FDistantSkyLightLutCS) FDistantSkyLightLutCSPtr;
