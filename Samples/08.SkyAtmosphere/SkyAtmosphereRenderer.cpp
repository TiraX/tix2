/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SkyAtmosphereRenderer.h"

FSkyAtmosphereRenderer::FSkyAtmosphereRenderer()
{
}

FSkyAtmosphereRenderer::~FSkyAtmosphereRenderer()
{
}

void FSkyAtmosphereRenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
}

void FSkyAtmosphereRenderer::InitInRenderThread()
{
	FDefaultRenderer::InitInRenderThread();

	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 RTWidth = 1600;
	const int32 RTHeight = TEngine::AppInfo.Height * RTWidth / TEngine::AppInfo.Width;

	TStreamPtr ArgumentValues = ti_new TStream;
	TVector<FTexturePtr> ArgumentTextures;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(RTWidth, RTHeight);
	RT_BasePass->SetResourceName("RT_BasePass");
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, 1, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, 1, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	// Init Lut Shaders
	TransmittanceCS = ti_new FTransmittanceLutCS();
	TransmittanceCS->Finalize();
	TransmittanceCS->PrepareResources(RHI);

	MeanIllumLutCS = ti_new FMeanIllumLutCS();
	MeanIllumLutCS->Finalize();
	MeanIllumLutCS->PrepareResources(RHI);

	DistantSkyLightLutCS = ti_new FDistantSkyLightLutCS();
	DistantSkyLightLutCS->Finalize();
	DistantSkyLightLutCS->PrepareResources(RHI);

	SkyViewLutCS = ti_new FSkyViewLutCS();
	SkyViewLutCS->Finalize();
	SkyViewLutCS->PrepareResources(RHI);

	CameraVolumeLutCS = ti_new FCameraVolumeLutCS();
	CameraVolumeLutCS->Finalize();
	CameraVolumeLutCS->PrepareResources(RHI);

	// Init Atmosphere uniform param
	const FViewProjectionInfo& ViewProjection = FRenderThread::Get()->GetRenderScene()->GetViewProjection();

	AtmosphereParam = ti_new FAtmosphereParam;
	AtmosphereParam->UniformBufferData[0].TransmittanceLutSizeAndInv = 
		FFloat4(float(FTransmittanceLutCS::LUT_W), float(FTransmittanceLutCS::LUT_H), 
		1.f / float(FTransmittanceLutCS::LUT_W), 1.f / float(FTransmittanceLutCS::LUT_H));
	AtmosphereParam->UniformBufferData[0].MultiScatteredLuminanceLutSizeAndInv =
		FFloat4(float(FMeanIllumLutCS::LUT_W), float(FMeanIllumLutCS::LUT_H),
			1.f / float(FMeanIllumLutCS::LUT_W), 1.f / float(FMeanIllumLutCS::LUT_H));
	AtmosphereParam->UniformBufferData[0].SkyViewLutSizeAndInv =
		FFloat4(float(FSkyViewLutCS::LUT_W), float(FSkyViewLutCS::LUT_H),
			1.f / float(FSkyViewLutCS::LUT_W), 1.f / float(FSkyViewLutCS::LUT_H));
	AtmosphereParam->UniformBufferData[0].CameraAerialPerspectiveVolumeSizeAndInv =
		FFloat4(float(FCameraVolumeLutCS::LUT_W), float(FCameraVolumeLutCS::LUT_H),
			1.f / float(FCameraVolumeLutCS::LUT_W), 1.f / float(FCameraVolumeLutCS::LUT_H));
	AtmosphereParam->UniformBufferData[0].ViewSizeAndInv =
		FFloat4(float(RTWidth), float(RTHeight),
			1.f / float(RTWidth), 1.f / float(RTHeight));

	// x = CameraAerialPerspectiveVolumeDepthResolution; y = CameraAerialPerspectiveVolumeDepthResolutionInv
	// z = CameraAerialPerspectiveVolumeDepthSliceLengthKm; w = CameraAerialPerspectiveVolumeDepthSliceLengthKmInv;
	AtmosphereParam->UniformBufferData[0].CameraAerialPerspectiveVolumeDepthInfo = 
		FFloat4(float(FCameraVolumeLutCS::LUT_D), 1.f / float(FCameraVolumeLutCS::LUT_D), 
			6.f, 1.f / 6.f);
	// x = AerialPerspectiveStartDepthKm; y = CameraAerialPerspectiveSampleCountPerSlice; z = AerialPespectiveViewDistanceScale;
	AtmosphereParam->UniformBufferData[0].AerialPerspectiveInfo = 
		FFloat4(0.1f, 1.f, 1.f, 1.f);

	const float TopRadiusKm = 6420.f;
	const float BottomRadiusKm = 6360.f;
	AtmosphereParam->UniformBufferData[0].RadiusRange = FFloat4(TopRadiusKm, BottomRadiusKm, sqrt(TopRadiusKm * TopRadiusKm - BottomRadiusKm * BottomRadiusKm), BottomRadiusKm * BottomRadiusKm);
	AtmosphereParam->UniformBufferData[0].MieRayleigh = FFloat4(0.8f, -0.83333f, -0.125f, 1.f);
	AtmosphereParam->UniformBufferData[0].Params1 = FFloat4(10.f, 1.f, 25.f, 15.f);
	AtmosphereParam->UniformBufferData[0].AbsorptionDensity01MA = FFloat4(0.06667f, -0.6667f, -0.06667f, 2.66667f);
	AtmosphereParam->UniformBufferData[0].MieScattering = FFloat4(0.004f, 0.004f, 0.004f, 0.004f);
	AtmosphereParam->UniformBufferData[0].MieAbsorption = FFloat4(0.00044f, 0.00044f, 0.00044f, 0.00044f);
	AtmosphereParam->UniformBufferData[0].MieExtinction = FFloat4(0.00444f, 0.00444f, 0.00444f, 0.00444f);
	AtmosphereParam->UniformBufferData[0].RayleighScattering = FFloat4(0.0058f, 0.01356f, 0.0331f, 1.f);
	AtmosphereParam->UniformBufferData[0].AbsorptionExtinction = FFloat4(0.00065f, 0.00188f, 0.00008f, 1.f);
	AtmosphereParam->UniformBufferData[0].GroundAlbedo = FFloat4(0.40198f, 0.40198f, 0.40198f, 1.f);
	AtmosphereParam->UniformBufferData[0].AtmosphereLightDirection0 = FFloat4(-0.96126f, 0.f, 0.27564f, 1.f);
	AtmosphereParam->UniformBufferData[0].AtmosphereLightDirection1 = FFloat4(0.125f, 0.08333f, 0.66667f, 0.00391f);
	AtmosphereParam->UniformBufferData[0].AtmosphereLightColor0 = FFloat4(10.f, 10.f, 10.f, 10.f);
	AtmosphereParam->UniformBufferData[0].AtmosphereLightColor1 = FFloat4(0.f, 0.f, 0.f, 0.f);
	AtmosphereParam->UniformBufferData[0].SkyLuminanceFactor = FFloat4(1.f, 1.f, 1.f, 1.f);
	AtmosphereParam->UniformBufferData[0].DistantSkyLightSampleAltitude = FFloat4(6.f, 0.f, 0.f, 0.f);
	AtmosphereParam->UniformBufferData[0].ViewForward = FFloat4(-0.99351f, 0.07306f, -0.08719f, 0.f);
	AtmosphereParam->UniformBufferData[0].ViewRight = FFloat4(-0.07334f, -0.99731f, -2.31944e-8f, 0.f);
	AtmosphereParam->UniformBufferData[0].SkySampleParam = FFloat4(4.f, 32.f, 0.00667f, 1.f);// x = FastSkySampleCountMin; y = FastSkySampleCountMax; z = FastSkyDistanceToSampleCountMaxInv; w = 1
	AtmosphereParam->UniformBufferData[0].SkyWorldCameraOrigin = FFloat4(ViewProjection.CamPos.X, ViewProjection.CamPos.Y, ViewProjection.CamPos.Z, 1.f);
	AtmosphereParam->UniformBufferData[0].SkyPlanetCenterAndViewHeight = FFloat4(0.f, 0.f, -6.36e6f, 6.36e6f);
	AtmosphereParam->UniformBufferData[0].View_AtmosphereLightDirection0 = FFloat4(-0.96126f, 0.f, 0.27564f, 1.f);
	AtmosphereParam->UniformBufferData[0].View_AtmosphereLightDirection1 = FFloat4(0.f, 0.f, 1.f, 1.f);
	AtmosphereParam->UniformBufferData[0].View_AtmosphereLightColor0 = FFloat4(10.f, 10.f, 10.f, 1.f);
	AtmosphereParam->UniformBufferData[0].View_AtmosphereLightColor1 = FFloat4(0.f, 0.f, 0.f, 0.f);
	AtmosphereParam->UniformBufferData[0].View_AtmosphereLightDiscLuminance0 = FFloat4(157601.32813f, 172847.79688f, 202683.09375f, 145911.29688f);
	AtmosphereParam->UniformBufferData[0].View_AtmosphereLightDiscCosHalfApexAngle0 = FFloat4(0.99999, 0.00, 0.00, 1.00);
	
	const float InvRTW = 1.f / RTWidth;
	const float InvRTH = 1.f / RTHeight;
	float Mx = 2.f * InvRTW;
	float My = -2.f * InvRTH;
	float Ax = -1.f;
	float Ay = 1.f;
	matrix4 mat(Mx, 0, 0, 0,
		0, My, 0, 0,
		0, 0, 1, 0,
		Ax, Ay, 0, 1);
	matrix4 MatVP = ViewProjection.MatProj * ViewProjection.MatView;
	matrix4 InvVP;
	MatVP.getInverse(InvVP);
	AtmosphereParam->UniformBufferData[0].SVPositionToTranslatedWorld = mat * InvVP;

	AtmosphereParam->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	// Load default pipeline
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetPtr DebugMaterialAsset = TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TResourcePtr DebugMaterialResource = DebugMaterialAsset->GetResourcePtr();
	TMaterialPtr DebugMaterial = static_cast<TMaterial*>(DebugMaterialResource.get());
	FPipelinePtr DebugPipeline = DebugMaterial->PipelineResource;

	// Load sky atmosphere pipeline
	const TString SkyMaterialName = "M_Sky.tasset";
	TAssetPtr SkyMaterialAsset = TAssetLibrary::Get()->LoadAsset(SkyMaterialName);
	TResourcePtr SkyMaterialResource = SkyMaterialAsset->GetResourcePtr();
	TMaterialPtr SkyMaterial = static_cast<TMaterial*>(SkyMaterialResource.get());
	SkyPipeline = SkyMaterial->PipelineResource;

	// Create sky argument buffer
	AB_SkyAtmosphere = RHI->CreateArgumentBuffer(3);
	{
		AB_SkyAtmosphere->SetTexture(0, TransmittanceCS->GetTransmittanceLutTexture());
		AB_SkyAtmosphere->SetTexture(1, SkyViewLutCS->GetSkyViewLut());
		AB_SkyAtmosphere->SetTexture(2, CameraVolumeLutCS->GetCameraAerialPerspectiveVolumeLut());
		RHI->UpdateHardwareResourceAB(AB_SkyAtmosphere, SkyPipeline->GetShader(), 0);
	}

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}
}

void FSkyAtmosphereRenderer::DrawSceneTiles(FRHI* RHI, FScene * Scene)
{
	const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
	for (auto& TileIter : SceneTileResources)
	{
		const vector2di& TilePos = TileIter.first;
		FSceneTileResourcePtr TileRes = TileIter.second;

		const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
		for (uint32 PIndex = 0; PIndex < (uint32)TilePrimitives.size(); ++PIndex)
		{
			FPrimitivePtr Primitive = TilePrimitives[PIndex];

			if (Primitive != nullptr)
			{
				FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
				RHI->SetGraphicsPipeline(Primitive->GetPipeline());
				RHI->SetMeshBuffer(Primitive->GetMeshBuffer(), InstanceBuffer);
				ApplyShaderParameter(RHI, Scene, Primitive);

				RHI->DrawPrimitiveIndexedInstanced(
					Primitive->GetMeshBuffer(),
					InstanceBuffer == nullptr ? 1 : Primitive->GetInstanceCount(),
					Primitive->GetInstanceOffset());
			}
		}
	}
}

void FSkyAtmosphereRenderer::Render(FRHI* RHI, FScene* Scene)
{
	int32 RTW = RHI->GetViewport().Width;
	int32 RTH = RHI->GetViewport().Height;
	const FViewProjectionInfo& ViewProjection = Scene->GetViewProjection();
	const float InvRTW = 1.f / RTW;
	const float InvRTH = 1.f / RTH;
	float Mx = 2.f * InvRTW;
	float My = -2.f * InvRTH;
	float Ax = -1.f;
	float Ay = 1.f;
	matrix4 mat(Mx, 0, 0, 0,
		0, My, 0, 0,
		0, 0, 1, 0,
		Ax, Ay, 0, 1);
	matrix4 MatVP = ViewProjection.MatProj * ViewProjection.MatView;
	matrix4 InvVP;
	MatVP.getInverse(InvVP);
	AtmosphereParam->UniformBufferData[0].SVPositionToTranslatedWorld = mat * InvVP;	
	
	matrix4 mat1(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, ViewProjection.MatProj[2 * 4 + 2], 1,
		0, 0, ViewProjection.MatProj[3 * 4 + 2], 0);
	matrix4 InvView;
	ViewProjection.MatView.getInverse(InvView);
	AtmosphereParam->UniformBufferData[0].ScreenToWorld = mat1 * InvView;

	float StartDepthZ = 0.1f;
	//if (bFastAerialPerspectiveDepthTest)
	{
		const matrix4& ProjectionMat = ViewProjection.MatProj;
		//const FMatrix ProjectionMatrix = View.ViewMatrices.GetProjectionMatrix();
		float HalfHorizontalFOV = atan(1.f / ProjectionMat[0]);
		//float HalfHorizontalFOV = FMath::Atan(1.0f / ProjectionMatrix.M[0][0]);
		float HalfVerticalFOV = atan(1.f / ProjectionMat[5]);
		//float HalfVerticalFOV = FMath::Atan(1.0f / ProjectionMatrix.M[1][1]);
		float StartDepthViewCm = cos(ti_max(HalfHorizontalFOV, HalfVerticalFOV)) * 0.1 * 1000 * 100;
		StartDepthViewCm = ti_max(StartDepthViewCm, 0.1f); // In any case, we need to limit the distance to frustum near plane to not be clipped away.
		FFloat4 Projected;
		ProjectionMat.transformVect(&Projected.X, vector3df(0.f, 0.f, StartDepthViewCm));
		//const FVector4 Projected = ProjectionMatrix.TransformFVector4(FVector4(0.0f, 0.0f, StartDepthViewCm, 1.0f));
		StartDepthZ = Projected.Z / Projected.W;
	}

	AtmosphereParam->UniformBufferData[0].SkyWorldCameraOrigin = FFloat4(ViewProjection.CamPos.X, ViewProjection.CamPos.Y, ViewProjection.CamPos.Z, StartDepthZ);


	AtmosphereParam->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	TransmittanceCS->UpdataComputeParams(RHI, AtmosphereParam->UniformBuffer);
	MeanIllumLutCS->UpdataComputeParams(RHI, AtmosphereParam->UniformBuffer, TransmittanceCS->GetTransmittanceLutTexture());
	DistantSkyLightLutCS->UpdataComputeParams(RHI, AtmosphereParam->UniformBuffer, TransmittanceCS->GetTransmittanceLutTexture(), MeanIllumLutCS->GetMultiScatteredLuminanceLut());
	SkyViewLutCS->UpdataComputeParams(RHI, AtmosphereParam->UniformBuffer, TransmittanceCS->GetTransmittanceLutTexture(), MeanIllumLutCS->GetMultiScatteredLuminanceLut());
	CameraVolumeLutCS->UpdataComputeParams(RHI, AtmosphereParam->UniformBuffer, TransmittanceCS->GetTransmittanceLutTexture(), MeanIllumLutCS->GetMultiScatteredLuminanceLut());

	RHI->BeginComputeTask();
	{
		RHI->BeginEvent("TransmittanceLut");
		TransmittanceCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("MeanIllumLut");
		MeanIllumLutCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("DistantSkyLightLut");
		DistantSkyLightLutCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("SkyViewLut");
		SkyViewLutCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("CameraVolumeLutCS");
		CameraVolumeLutCS->Run(RHI);
		RHI->EndEvent();
	}
	RHI->EndComputeTask();

	RHI->BeginRenderToRenderTarget(RT_BasePass, 0, "BasePass");
	DrawSceneTiles(RHI, Scene);
	{
		// Draw sky full screen quad
		RHI->SetGraphicsPipeline(SkyPipeline);
		RHI->SetUniformBuffer(ESS_PIXEL_SHADER, 0, AtmosphereParam->UniformBuffer);
		RHI->SetArgumentBuffer(1, AB_SkyAtmosphere);
		FSRender.DrawFullScreenQuad(RHI);
	}

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
