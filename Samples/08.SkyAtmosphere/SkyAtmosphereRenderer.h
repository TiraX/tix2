/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TransmittanceLutCS.h"
#include "MeanIllumLutCS.h"
#include "DistantSkyLightLut.h"
#include "SkyViewLut.h"
#include "CameraVolumeLut.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FAtmosphereParam)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, TransmittanceLutSizeAndInv)	// xy = Size; zw = InvSize;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MultiScatteredLuminanceLutSizeAndInv)	// xy = Size; zw = InvSize;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, SkyViewLutSizeAndInv)	// xy = Size; zw = InvSize;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, CameraAerialPerspectiveVolumeSizeAndInv)	// xy = Size; zw = InvSize;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, ViewSizeAndInv)	// xy = Size; zw = InvSize;
	// x = CameraAerialPerspectiveVolumeDepthResolution; y = CameraAerialPerspectiveVolumeDepthResolutionInv
	// z = CameraAerialPerspectiveVolumeDepthSliceLengthKm; w = CameraAerialPerspectiveVolumeDepthSliceLengthKmInv;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, CameraAerialPerspectiveVolumeDepthInfo)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AerialPerspectiveInfo)	// x = AerialPerspectiveStartDepthKm; y = CameraAerialPerspectiveSampleCountPerSlice; z = AerialPespectiveViewDistanceScale;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, RadiusRange)	// x = TopRadiusKm; y = BottomRadiusKm; z = sqrt(x * x - y * y); w = y * y;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieRayleigh)		// x = MiePhaseG; y = MieDensityExpScale; z = RayleighDensityExpScale; w = MultiScatteringFactor;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Params1)		// x = TransmittanceSampleCount; y = ViewPreExposure; z = AbsorptionDensity0LayerWidth; w = MultiScatteringSampleCount
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AbsorptionDensity01MA)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieScattering)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieAbsorption)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, MieExtinction)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, RayleighScattering)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AbsorptionExtinction)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, GroundAlbedo)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AtmosphereLightDirection0)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AtmosphereLightDirection1)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AtmosphereLightColor0)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, AtmosphereLightColor1)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, SkyLuminanceFactor)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, DistantSkyLightSampleAltitude)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, ViewForward)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, ViewRight)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, SkySampleParam)	// x = FastSkySampleCountMin; y = FastSkySampleCountMax; z = FastSkyDistanceToSampleCountMaxInv; w = PreExposure
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, SkyWorldCameraOrigin)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, SkyPlanetCenterAndViewHeight)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, View_AtmosphereLightDirection0)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, View_AtmosphereLightDirection1)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, View_AtmosphereLightColor0)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, View_AtmosphereLightColor1)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, View_AtmosphereLightDiscLuminance0)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, View_AtmosphereLightDiscCosHalfApexAngle0)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, SVPositionToTranslatedWorld)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, ScreenToWorld)
END_UNIFORM_BUFFER_STRUCT(FAtmosphereParam)

class FSkyAtmosphereRenderer : public FDefaultRenderer
{
public:
	FSkyAtmosphereRenderer();
	virtual ~FSkyAtmosphereRenderer();

	static FSkyAtmosphereRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	void DrawSceneTiles(FRHI* RHI, FScene * Scene);

private:
	FPipelinePtr SkyPipeline;
	FArgumentBufferPtr AB_SkyAtmosphere;
	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FAtmosphereParamPtr AtmosphereParam;

	// Lut Compute Shaders
	FTransmittanceLutCSPtr TransmittanceCS;
	FMeanIllumLutCSPtr MeanIllumLutCS;
	FDistantSkyLightLutCSPtr DistantSkyLightLutCS;
	FSkyViewLutCSPtr SkyViewLutCS;
	FCameraVolumeLutCSPtr CameraVolumeLutCS;
};
