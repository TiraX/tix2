/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SSSSRenderer.h"
#include "SeparableSSS.h"

FS4TempRenderer::FS4TempRenderer()
{
}

FS4TempRenderer::~FS4TempRenderer()
{
}

void FS4TempRenderer::InitInRenderThread()
{
}

void FS4TempRenderer::Render(FRHI* RHI, FScene* Scene)
{
    _LOG(Log, "FS4TempRenderer .\n");
}

FSSSSRenderer::FSSSSRenderer()
{
	S4Effect = ti_new SeparableSSS(DEG_TO_RAD(40), 250.f);

	const TString SSSBlurMaterialName = "M_SSSBlur.tasset";
	TMaterialPtr M_SSSBlur = static_cast<TMaterial*>(TAssetLibrary::Get()->LoadAsset(SSSBlurMaterialName)->GetResourcePtr());
	PL_SSSBlur = M_SSSBlur->PipelineResource;

	const TString AddSpecularMaterialName = "M_AddSpecular.tasset";
	TMaterialPtr M_AddSpecular = static_cast<TMaterial*>(TAssetLibrary::Get()->LoadAsset(AddSpecularMaterialName)->GetResourcePtr());
	PL_AddSpecular = M_AddSpecular->PipelineResource;

	const TString GlareDetectionMaterialName = "M_GlareDetection.tasset";
	TMaterialPtr M_GlareDetection = static_cast<TMaterial*>(TAssetLibrary::Get()->LoadAsset(GlareDetectionMaterialName)->GetResourcePtr());
	PL_GlareDetection = M_GlareDetection->PipelineResource;
	
	const TString BloomMaterialName = "M_Bloom.tasset";
	TMaterialPtr M_Bloom = static_cast<TMaterial*>(TAssetLibrary::Get()->LoadAsset(BloomMaterialName)->GetResourcePtr());
	PL_Bloom = M_Bloom->PipelineResource;

	const TString CombineMaterialName = "M_Combine.tasset";
	TMaterialPtr M_Combine = static_cast<TMaterial*>(TAssetLibrary::Get()->LoadAsset(CombineMaterialName)->GetResourcePtr());
	PL_Combine = M_Combine->PipelineResource;
}

FSSSSRenderer::~FSSSSRenderer()
{
	ti_delete S4Effect;
	PL_SSSBlur = nullptr;
	RT_BasePass = nullptr;
	RT_SSSBlurX = nullptr;
	RT_SSSBlurY = nullptr;
	PL_AddSpecular = nullptr;
}

void FSSSSRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 ViewWidth = 1600;
    const int32 ViewHeight = TEngine::AppInfo.Height * ViewWidth / TEngine::AppInfo.Width;

	TStreamPtr ArgumentValues = ti_new TStream;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_BasePass->SetResourceName("BasePass");
#endif
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR1, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->Compile();

	FTexturePtr SceneColor = RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture;
	FTexturePtr SceneDepth = RT_BasePass->GetDepthStencilBuffer().Texture;
	FTexturePtr SpecularTex = RT_BasePass->GetColorBuffer(ERTC_COLOR1).Texture;
	
	// Setup SSS blur pass render targets and texture tables
	// RT BlurX
	RT_SSSBlurX = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_SSSBlurX->SetResourceName("SSSBlurX");
#endif
	RT_SSSBlurX->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_DONTCARE, ERT_STORE_STORE);
	RT_SSSBlurX->AddDepthStencilBuffer(SceneDepth, ERT_LOAD_LOAD, ERT_STORE_DONTCARE);
	RT_SSSBlurX->Compile();

	// RT BlurY
	RT_SSSBlurY = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_SSSBlurY->SetResourceName("SSSBlurY");
#endif
	RT_SSSBlurY->AddColorBuffer(SceneColor, ERTC_COLOR0, ERT_LOAD_DONTCARE, ERT_STORE_STORE);
	RT_SSSBlurY->AddDepthStencilBuffer(SceneDepth, ERT_LOAD_LOAD, ERT_STORE_DONTCARE);
	RT_SSSBlurY->Compile();
	AB_SSSBlurX = FRHI::Get()->CreateArgumentBuffer(2);
	AB_SSSBlurY = FRHI::Get()->CreateArgumentBuffer(2);
	{
		float ar = (float)ViewHeight / (float)ViewWidth;
		ArgumentValues->Reset();
		FFloat4 BlurDir, BlurParam;
		FFloat4 Kernel[SeparableSSS::SampleCount];
		BlurDir = FFloat4(ar, 0.f, 0.f, 0.f);
		BlurParam = FFloat4(S4Effect->getWidth(), S4Effect->getFOV(), S4Effect->getMaxOffset(), 0.f);

		const TVector<vector4df>& KernelData = S4Effect->getKernel();
		TI_ASSERT(KernelData.size() == SeparableSSS::SampleCount);
		for (int32 i = 0; i < SeparableSSS::SampleCount; ++i)
		{
			Kernel[i] = KernelData[i];
		}
		ArgumentValues->Put(&BlurDir, sizeof(FFloat4));
		ArgumentValues->Put(&BlurParam, sizeof(FFloat4));
		ArgumentValues->Put(&Kernel, sizeof(FFloat4) * SeparableSSS::SampleCount);
		AB_SSSBlurX->SetDataBuffer(ArgumentValues->GetBuffer(), ArgumentValues->GetLength());
		AB_SSSBlurX->SetTexture(0, SceneColor);
		AB_SSSBlurX->SetTexture(1, SpecularTex);
		RHI->UpdateHardwareResourceAB(AB_SSSBlurX);

		ArgumentValues->Reset();
		BlurDir = FFloat4(0.f, 1.f, 0.f, 0.f);
		ArgumentValues->Put(&BlurDir, sizeof(FFloat4));
		ArgumentValues->Put(&BlurParam, sizeof(FFloat4));
		ArgumentValues->Put(&Kernel, sizeof(FFloat4) * SeparableSSS::SampleCount);
		AB_SSSBlurY->SetDataBuffer(ArgumentValues->GetBuffer(), ArgumentValues->GetLength());

		FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
		AB_SSSBlurY->SetTexture(0, TextureBlurX);
		AB_SSSBlurY->SetTexture(1, SpecularTex);
		RHI->UpdateHardwareResourceAB(AB_SSSBlurY);
	}

	// AB_AddSpecular
	{
		AB_AddSpecular = FRHI::Get()->CreateArgumentBuffer(2);
		AB_AddSpecular->SetTexture(0, SceneColor);
		AB_AddSpecular->SetTexture(1, SpecularTex);
		RHI->UpdateHardwareResourceAB(AB_AddSpecular);
	}

	const float Exposure = 2.f;
	const float Threshold = 0.63f;
	// Bloom Glare Detection
	RT_GlareDetection = FRenderTarget::Create(ViewWidth / 2, ViewHeight / 2);
	RT_GlareDetection->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_DONTCARE, ERT_STORE_STORE);
	RT_GlareDetection->Compile();
	{
		ArgumentValues->Reset();
		FFloat4 BloomParam;
		BloomParam.X = Exposure;
		BloomParam.Y = Threshold;
		ArgumentValues->Put(&BloomParam, sizeof(FFloat4));

		AB_GlareDetection = FRHI::Get()->CreateArgumentBuffer(1);
		AB_GlareDetection->SetDataBuffer(ArgumentValues->GetBuffer(), ArgumentValues->GetLength());
		FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
		AB_GlareDetection->SetTexture(0, TextureBlurX);
		RHI->UpdateHardwareResourceAB(AB_GlareDetection);
	}

	// Bloom Blur Passes
	float BloomWidth = 1.f;
	int32 SizeBase = 4;
	FTexturePtr LastResult = RT_GlareDetection->GetColorBuffer(ERTC_COLOR0).Texture;
	for (int32 p = 0; p < BloomPasses; ++p)
	{
		vector2df BloomStep = vector2df(1.f / (ViewWidth / SizeBase), 1.f / (ViewHeight / SizeBase));
		BloomStep *= BloomWidth;

		FFloat4 BloomParam;
		// Pass Horizontal
		FBloomPass& PassX = BloomPass[p][0];
		PassX.RT = FRenderTarget::Create(ViewWidth / SizeBase, ViewHeight / SizeBase);
		PassX.RT->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_DONTCARE, ERT_STORE_STORE);
		PassX.RT->Compile();
		PassX.AB = FRHI::Get()->CreateArgumentBuffer(1);
		vector2df BloomStepX = BloomStep * vector2df(1.f, 0.f);
		BloomParam = BloomStepX;
		{
			PassX.AB->SetDataBuffer(&BloomParam, sizeof(FFloat4));
			PassX.AB->SetTexture(0, LastResult);
            RHI->UpdateHardwareResourceAB(PassX.AB);
		}
		LastResult = PassX.RT->GetColorBuffer(ERTC_COLOR0).Texture;

		// Pass Vertical
		FBloomPass& PassY = BloomPass[p][1];
		PassY.RT = FRenderTarget::Create(ViewWidth / SizeBase, ViewHeight / SizeBase);
		PassY.RT->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_DONTCARE, ERT_STORE_STORE);
		PassY.RT->Compile();
		PassY.AB = FRHI::Get()->CreateArgumentBuffer(1);
		vector2df BloomStepY = BloomStep * vector2df(0.f, 1.f);
		BloomParam = BloomStepY;
		{
			PassY.AB->SetDataBuffer(&BloomParam, sizeof(FFloat4));
			PassY.AB->SetTexture(0, LastResult);
            RHI->UpdateHardwareResourceAB(PassY.AB);
		}
		LastResult = PassY.RT->GetColorBuffer(ERTC_COLOR0).Texture;

		SizeBase *= 2;
	}

	const float BloomIntensity = 1.f;
	// Combine result
	RT_Combine = FRenderTarget::Create(ViewWidth, ViewHeight);
	RT_Combine->AddColorBuffer(EPF_RGBA8, ERTC_COLOR0, ERT_LOAD_DONTCARE, ERT_STORE_STORE);
	RT_Combine->Compile();
	{
		FFloat4 BloomParam;
		BloomParam.X = Exposure;
		BloomParam.Y = BloomIntensity;

		AB_Combine = FRHI::Get()->CreateArgumentBuffer(3);
		AB_Combine->SetDataBuffer(&BloomParam, sizeof(FFloat4));
		FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
		AB_Combine->SetTexture(0, TextureBlurX);
		AB_Combine->SetTexture(1, BloomPass[0][1].RT->GetColorBuffer(ERTC_COLOR0).Texture);
		AB_Combine->SetTexture(2, BloomPass[1][1].RT->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Combine);
	}

	// Output result
    AB_Result = RHI->CreateArgumentBuffer(1);
    {
		AB_Result->SetTexture(0, RT_Combine->GetColorBuffer(ERTC_COLOR0).Texture);
        RHI->UpdateHardwareResourceAB(AB_Result);
    }
	ArgumentValues = nullptr;
}

void FSSSSRenderer::Render(FRHI* RHI, FScene* Scene)
{
	// Render Base Pass
	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	RenderDrawList(RHI, Scene, LIST_OPAQUE);
    

	// Go SSS Blur Pass
	{
		RHI->BeginRenderToRenderTarget(RT_SSSBlurX, "SSSBlueX");
		RHI->SetGraphicsPipeline(PL_SSSBlur);
		ApplyShaderParameter(RHI, PL_SSSBlur->GetShader(), Scene, AB_SSSBlurX);
		FSRender.DrawFullScreenQuad(RHI);
	}
	{
		RHI->BeginRenderToRenderTarget(RT_SSSBlurY, "SSSBlurY");
        RHI->SetGraphicsPipeline(PL_SSSBlur);
		ApplyShaderParameter(RHI, PL_SSSBlur->GetShader(), Scene, AB_SSSBlurY);
		FSRender.DrawFullScreenQuad(RHI);
	}

	// Add Specular
	{
		RHI->BeginRenderToRenderTarget(RT_SSSBlurX, "AddSpecular");
		RHI->SetGraphicsPipeline(PL_AddSpecular);
		ApplyShaderParameter(RHI, PL_AddSpecular->GetShader(), Scene, AB_AddSpecular);
		FSRender.DrawFullScreenQuad(RHI);
	}

	// Bloom
	{
		// Glare detection
		RHI->BeginRenderToRenderTarget(RT_GlareDetection, "GlareDetection");
		RHI->SetGraphicsPipeline(PL_GlareDetection);
		ApplyShaderParameter(RHI, PL_GlareDetection->GetShader(), Scene, AB_GlareDetection);
		FSRender.DrawFullScreenQuad(RHI);

		// Bloom blur X & Y
		for (int32 p = 0; p < BloomPasses; ++p)
		{
			// Pass Horizontal
			FBloomPass& PassX = BloomPass[p][0];
			RHI->BeginRenderToRenderTarget(PassX.RT, "BloomX");
			RHI->SetGraphicsPipeline(PL_Bloom);
			ApplyShaderParameter(RHI, PL_Bloom->GetShader(), Scene, PassX.AB);
			FSRender.DrawFullScreenQuad(RHI);
			
			// Pass Vertical
			FBloomPass& PassY = BloomPass[p][1];
			RHI->BeginRenderToRenderTarget(PassY.RT, "BloomY");
			RHI->SetGraphicsPipeline(PL_Bloom);
			ApplyShaderParameter(RHI, PL_Bloom->GetShader(), Scene, PassY.AB);
			FSRender.DrawFullScreenQuad(RHI);
		}
	}

	// Combine
	{
		RHI->BeginRenderToRenderTarget(RT_Combine, "Combine");
		RHI->SetGraphicsPipeline(PL_Combine);
		ApplyShaderParameter(RHI, PL_Combine->GetShader(), Scene, AB_Combine);
		FSRender.DrawFullScreenQuad(RHI);
	}
    
    RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
