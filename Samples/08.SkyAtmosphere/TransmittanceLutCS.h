/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

BEGIN_UNIFORM_BUFFER_STRUCT(FAtomsphereParam)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, TransmittanceLutSizeAndInv)	// xy = Size; zw = InvSize;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, RadiusRange)	// x = TopRadiusKm; y = BottomRadiusKm; z = sqrt(x * x - y * y); w = y * y;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieRayleigh)	// x = MiePhaseG; y = MieDensityExpScale; z = RayleighDensityExpScale; w = 1;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Params1)	// x = TransmittanceSampleCount; y = ViewPreExposure; z = AbsorptionDensity0LayerWidth; w = 1
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AbsorptionDensity01MA)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieScattering)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieAbsorption)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieExtinction)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, RayleighScattering)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AbsorptionExtinction)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, GroundAlbedo)
END_UNIFORM_BUFFER_STRUCT(FAtomsphereParam)

class FTransmittanceLutCS : public FComputeTask
{
public:
	static const int32 LUT_W = 256;
	static const int32 LUT_H = 64;

	FTransmittanceLutCS();
	virtual ~FTransmittanceLutCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

private:
	enum
	{
		UAV_LUT_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	FAtomsphereParamPtr AtomsphereParam;
	FTexturePtr TransmittanceLut;

};
typedef TI_INTRUSIVE_PTR(FTransmittanceLutCS) FTransmittanceLutCSPtr;
