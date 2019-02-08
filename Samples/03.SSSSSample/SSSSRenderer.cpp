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

	const TString SSSBlurMaterialName = "M_SSSBlur.tres";
	TMaterialPtr M_SSSBlur = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(SSSBlurMaterialName).get());
	PL_SSSBlur = M_SSSBlur->PipelineResource;
	AB_SSSBlurX = FRHI::Get()->CreateArgumentBuffer(M_SSSBlur->GetDesc().Shader->ShaderResource);
	AB_SSSBlurY = FRHI::Get()->CreateArgumentBuffer(M_SSSBlur->GetDesc().Shader->ShaderResource);

	const TString AddSpecularMaterialName = "M_AddSpecular.tres";
	TMaterialPtr M_AddSpecular = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(AddSpecularMaterialName).get());
	PL_AddSpecular = M_AddSpecular->PipelineResource;
	AB_AddSpecular = FRHI::Get()->CreateArgumentBuffer(M_AddSpecular->GetDesc().Shader->ShaderResource);

	const TString GlareDetectionMaterialName = "M_GlareDetection.tres";
	TMaterialPtr M_GlareDetection = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(GlareDetectionMaterialName).get());
	PL_GlareDetection = M_GlareDetection->PipelineResource;
	AB_GlareDetection = FRHI::Get()->CreateArgumentBuffer(M_GlareDetection->GetDesc().Shader->ShaderResource);

	const TString BloomMaterialName = "M_Bloom.tres";
	TMaterialPtr M_Bloom = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(BloomMaterialName).get());
	PL_Bloom = M_Bloom->PipelineResource;
	for (int32 p = 0; p < BloomPasses; ++p)
	{
		// Pass Horizontal
		FBloomPass& PassX = BloomPass[p][0];
		PassX.AB = FRHI::Get()->CreateArgumentBuffer(M_Bloom->GetDesc().Shader->ShaderResource);

		// Pass Vertical
		FBloomPass& PassY = BloomPass[p][1];
		PassY.AB = FRHI::Get()->CreateArgumentBuffer(M_Bloom->GetDesc().Shader->ShaderResource);
	}

	const TString CombineMaterialName = "M_Combine.tres";
	TMaterialPtr M_Combine = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(CombineMaterialName).get());
	PL_Combine = M_Combine->PipelineResource;
	AB_Combine = FRHI::Get()->CreateArgumentBuffer(M_Combine->GetDesc().Shader->ShaderResource);
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
	TVector<FTexturePtr> ArgumentTextures;

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

		ArgumentTextures.clear();
		ArgumentTextures.push_back(SceneColor);
		ArgumentTextures.push_back(SpecularTex);
		RHI->UpdateHardwareResource(AB_SSSBlurX, ArgumentValues, ArgumentTextures);

		ArgumentValues->Reset();
		BlurDir = FFloat4(0.f, 1.f, 0.f, 0.f);
		ArgumentValues->Put(&BlurDir, sizeof(FFloat4));
		ArgumentValues->Put(&BlurParam, sizeof(FFloat4));
		ArgumentValues->Put(&Kernel, sizeof(FFloat4) * SeparableSSS::SampleCount);

		ArgumentTextures.clear();
		FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
		ArgumentTextures.push_back(TextureBlurX);
		ArgumentTextures.push_back(SpecularTex);
		RHI->UpdateHardwareResource(AB_SSSBlurY, ArgumentValues, ArgumentTextures);
	}

	// AB_AddSpecular
	{
		ArgumentTextures.clear();
		ArgumentTextures.push_back(SceneColor);
		ArgumentTextures.push_back(SpecularTex);
		RHI->UpdateHardwareResource(AB_AddSpecular, nullptr, ArgumentTextures);
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

		FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
		ArgumentTextures.clear();
		ArgumentTextures.push_back(TextureBlurX);
		RHI->UpdateHardwareResource(AB_GlareDetection, ArgumentValues, ArgumentTextures);
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
		vector2df BloomStepX = BloomStep * vector2df(1.f, 0.f);
		BloomParam = BloomStepX;
		{
			ArgumentValues->Reset();
			ArgumentValues->Put(&BloomParam, sizeof(FFloat4));
			ArgumentTextures.clear();
			ArgumentTextures.push_back(LastResult);
            RHI->UpdateHardwareResource(PassX.AB, ArgumentValues, ArgumentTextures);
		}
		LastResult = PassX.RT->GetColorBuffer(ERTC_COLOR0).Texture;

		// Pass Vertical
		FBloomPass& PassY = BloomPass[p][1];
		PassY.RT = FRenderTarget::Create(ViewWidth / SizeBase, ViewHeight / SizeBase);
		PassY.RT->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_DONTCARE, ERT_STORE_STORE);
		PassY.RT->Compile();
		vector2df BloomStepY = BloomStep * vector2df(0.f, 1.f);
		BloomParam = BloomStepY;
		{
			ArgumentValues->Reset();
			ArgumentValues->Put(&BloomParam, sizeof(FFloat4));
			ArgumentTextures.clear();
			ArgumentTextures.push_back(LastResult);
            RHI->UpdateHardwareResource(PassY.AB, ArgumentValues, ArgumentTextures);
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
		ArgumentValues->Reset();
		FFloat4 BloomParam;
		BloomParam.X = Exposure;
		BloomParam.Y = BloomIntensity;
		ArgumentValues->Put(&BloomParam, sizeof(FFloat4));

		FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
		ArgumentTextures.clear();
		ArgumentTextures.push_back(TextureBlurX);
		ArgumentTextures.push_back(BloomPass[0][1].RT->GetColorBuffer(ERTC_COLOR0).Texture);
		ArgumentTextures.push_back(BloomPass[1][1].RT->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResource(AB_Combine, ArgumentValues, ArgumentTextures);
	}

	// Output result
    AB_Result = RHI->CreateArgumentBuffer(FSRender.GetFullScreenShader());
    {
        ArgumentValues->Reset();
        ArgumentTextures.clear();
        ArgumentTextures.push_back(RT_Combine->GetColorBuffer(ERTC_COLOR0).Texture);
        RHI->UpdateHardwareResource(AB_Result, ArgumentValues, ArgumentTextures);
    }
}

void FSSSSRenderer::Render(FRHI* RHI, FScene* Scene)
{
	Scene->PrepareViewUniforms();
    
	// Render Base Pass
	RHI->PushRenderTarget(RT_BasePass, "BasePass");

	RHI->SetStencilRef(1);

	const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList();
	for (const auto& Primitive : Primitives)
	{
		for (int32 m = 0; m < (int32)Primitive->MeshBuffers.size(); ++m)
		{
			FMeshBufferPtr MB = Primitive->MeshBuffers[m];
			FPipelinePtr PL = Primitive->Pipelines[m];

			{
                RHI->SetPipeline(PL);
				RHI->SetMeshBuffer(MB);
				ApplyShaderParameter(RHI, Scene, Primitive, m);
				RHI->DrawPrimitiveIndexedInstanced(MB, 1);
			}
		}
	}

	RHI->PopRenderTarget();
    

	// Go SSS Blur Pass
	{
		RHI->PushRenderTarget(RT_SSSBlurX, "SSSBlueX");
		RHI->SetPipeline(PL_SSSBlur);
		ApplyShaderParameter(RHI, PL_SSSBlur->GetShader(), Scene, AB_SSSBlurX);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}
	{
		RHI->PushRenderTarget(RT_SSSBlurY, "SSSBlurY");
        RHI->SetPipeline(PL_SSSBlur);
		ApplyShaderParameter(RHI, PL_SSSBlur->GetShader(), Scene, AB_SSSBlurY);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}

	// Add Specular
	{
		RHI->PushRenderTarget(RT_SSSBlurX, "AddSpecular");
		RHI->SetPipeline(PL_AddSpecular);
		ApplyShaderParameter(RHI, PL_AddSpecular->GetShader(), Scene, AB_AddSpecular);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}

	// Bloom
	{
		// Glare detection
		RHI->PushRenderTarget(RT_GlareDetection, "GlareDetection");
		RHI->SetPipeline(PL_GlareDetection);
		ApplyShaderParameter(RHI, PL_GlareDetection->GetShader(), Scene, AB_GlareDetection);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();

		// Bloom blur X & Y
		for (int32 p = 0; p < BloomPasses; ++p)
		{
			// Pass Horizontal
			FBloomPass& PassX = BloomPass[p][0];
			RHI->PushRenderTarget(PassX.RT, "BloomX");
			RHI->SetPipeline(PL_Bloom);
			ApplyShaderParameter(RHI, PL_Bloom->GetShader(), Scene, PassX.AB);
			FSRender.DrawFullScreenQuad(RHI);
			RHI->PopRenderTarget();
			
			// Pass Vertical
			FBloomPass& PassY = BloomPass[p][1];
			RHI->PushRenderTarget(PassY.RT, "BloomY");
			RHI->SetPipeline(PL_Bloom);
			ApplyShaderParameter(RHI, PL_Bloom->GetShader(), Scene, PassY.AB);
			FSRender.DrawFullScreenQuad(RHI);
			RHI->PopRenderTarget();
		}
	}

	// Combine
	{
		RHI->PushRenderTarget(RT_Combine, "Combine");
		RHI->SetPipeline(PL_Combine);
		ApplyShaderParameter(RHI, PL_Combine->GetShader(), Scene, AB_Combine);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}
    
    RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
