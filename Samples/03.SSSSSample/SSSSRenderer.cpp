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
	S4Effect = ti_new SeparableSSS(1600, 900, DEG_TO_RAD(40), 250.f);

	const TString SSSBlurMaterialName = "M_SSSBlur.tres";
	TMaterialPtr M_SSSBlur = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(SSSBlurMaterialName).get());
	PL_SSSBlur = M_SSSBlur->PipelineResource;

	const TString AddSpecularMaterialName = "M_AddSpecular.tres";
	TMaterialPtr M_AddSpecular = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(AddSpecularMaterialName).get());
	PL_AddSpecular = M_AddSpecular->PipelineResource;

	const TString GlareDetectionMaterialName = "M_GlareDetection.tres";
	TMaterialPtr M_GlareDetection = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(GlareDetectionMaterialName).get());
	PL_GlareDetection = M_GlareDetection->PipelineResource;

	const TString BloomMaterialName = "M_Bloom.tres";
	TMaterialPtr M_Bloom = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(BloomMaterialName).get());
	PL_Bloom = M_Bloom->PipelineResource;

	const TString CombineMaterialName = "M_Combine.tres";
	TMaterialPtr M_Combine = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(CombineMaterialName).get());
	PL_Combine = M_Combine->PipelineResource;
}

FSSSSRenderer::~FSSSSRenderer()
{
	ti_delete S4Effect;
	PL_SSSBlur = nullptr;
	RT_BasePass = nullptr;
	RT_SSSBlurX = nullptr;
	RT_SSSBlurY = nullptr;
	TT_SSSBlurX = nullptr;
	TT_SSSBlurY = nullptr;
	UB_SSSBlurX = nullptr;
	UB_SSSBlurY = nullptr;
	UB_Kernel = nullptr;
	PL_AddSpecular = nullptr;
}

void FSSSSRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 ViewWidth = 1600;
	const int32 ViewHeight = 900;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_BasePass->SetResourceName("BasePass");
#endif
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0);
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR1);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8);
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
	RT_SSSBlurX->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0);
	RT_SSSBlurX->AddDepthStencilBuffer(SceneDepth);
	RT_SSSBlurX->Compile();

	// TT BlurX
	TT_SSSBlurX = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(3);
	TT_SSSBlurX->PutTextureInTable(SceneColor, 0);
	TT_SSSBlurX->PutTextureInTable(SceneDepth, 1);
	TT_SSSBlurX->PutTextureInTable(SpecularTex, 2);

	// RT BlurY
	RT_SSSBlurY = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_SSSBlurY->SetResourceName("SSSBlurY");
#endif
	RT_SSSBlurY->AddColorBuffer(SceneColor, ERTC_COLOR0);
	RT_SSSBlurY->AddDepthStencilBuffer(SceneDepth);
	RT_SSSBlurY->Compile();

	// TT BlurY
	FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
	TT_SSSBlurY = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(3);
	TT_SSSBlurY->PutTextureInTable(TextureBlurX, 0);
	TT_SSSBlurY->PutTextureInTable(SceneDepth, 1);
	TT_SSSBlurY->PutTextureInTable(SpecularTex, 2);

	// Uniform buffers
	//x = sssWidth; y = sssFov; z = maxOffsetMm
	float ar = (float)ViewHeight / (float)ViewWidth;
	UB_SSSBlurX = ti_new FSSSBlurUniformBuffer;
	UB_SSSBlurX->UniformBufferData.BlurDir = FFloat4(ar, 0.f, 0.f, 0.f);
	UB_SSSBlurX->UniformBufferData.BlurParam = FFloat4(S4Effect->getWidth(), S4Effect->getFOV(), S4Effect->getMaxOffset(), 0.f);
	UB_SSSBlurX->InitUniformBuffer();

	UB_SSSBlurY = ti_new FSSSBlurUniformBuffer;
	UB_SSSBlurY->UniformBufferData.BlurDir = FFloat4(0.f, 1.f, 0.f, 0.f);
	UB_SSSBlurY->UniformBufferData.BlurParam = FFloat4(S4Effect->getWidth(), S4Effect->getFOV(), S4Effect->getMaxOffset(), 0.f);
	UB_SSSBlurY->InitUniformBuffer();

	// Fill kernel uniform buffer
	UB_Kernel = ti_new FSSSBlurKernelUniformBuffer;
	const TVector<vector4df>& KernelData = S4Effect->getKernel();
	TI_ASSERT(KernelData.size() == SeparableSSS::SampleCount);
	for (int32 i = 0 ; i < SeparableSSS::SampleCount; ++ i)
	{
		UB_Kernel->UniformBufferData.Kernel[i] = KernelData[i];
	}
	UB_Kernel->InitUniformBuffer();

	const float Exposure = 2.f;
	const float Threshold = 0.63f;
	// Bloom Glare Detection
	RT_GlareDetection = FRenderTarget::Create(ViewWidth / 2, ViewHeight / 2);
	RT_GlareDetection->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0);
	RT_GlareDetection->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8);
	RT_GlareDetection->Compile();
	UB_GlareParam = ti_new FSSSBloomUniformBuffer;
	UB_GlareParam->UniformBufferData.BloomParam.X = Exposure;
	UB_GlareParam->UniformBufferData.BloomParam.Y = Threshold;
	UB_GlareParam->InitUniformBuffer();
	TT_GlareSource = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(1);
	TT_GlareSource->PutTextureInTable(TextureBlurX, 0);

	// Bloom Blur Passes
	float BloomWidth = 1.f;
	int32 SizeBase = 4;
	FTexturePtr LastResult = RT_GlareDetection->GetColorBuffer(ERTC_COLOR0).Texture;
	for (int32 p = 0; p < BloomPasses; ++p)
	{
		vector2df BloomStep = vector2df(1.f / (ViewWidth / SizeBase), 1.f / (ViewHeight / SizeBase));
		BloomStep *= BloomWidth;

		// Pass Horizontal
		FBloomPass& PassX = BloomPass[p][0];
		PassX.RT = FRenderTarget::Create(ViewWidth / SizeBase, ViewHeight / SizeBase);
		PassX.RT->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0);
		PassX.RT->Compile();
		vector2df BloomStepX = BloomStep * vector2df(1.f, 0.f);
		PassX.UB = ti_new FSSSBloomUniformBuffer;
		PassX.UB->UniformBufferData.BloomParam = BloomStepX;
		PassX.UB->InitUniformBuffer();
		PassX.TT = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(1);
		PassX.TT->PutTextureInTable(LastResult, 0);
		LastResult = PassX.RT->GetColorBuffer(ERTC_COLOR0).Texture;

		// Pass Vertical
		FBloomPass& PassY = BloomPass[p][1];
		PassY.RT = FRenderTarget::Create(ViewWidth / SizeBase, ViewHeight / SizeBase);
		PassY.RT->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0);
		PassY.RT->Compile();
		vector2df BloomStepY = BloomStep * vector2df(0.f, 1.f);
		PassY.UB = ti_new FSSSBloomUniformBuffer;
		PassY.UB->UniformBufferData.BloomParam = BloomStepY;
		PassY.UB->InitUniformBuffer();
		PassY.TT = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(1);
		PassY.TT->PutTextureInTable(LastResult, 0);
		LastResult = PassY.RT->GetColorBuffer(ERTC_COLOR0).Texture;

		SizeBase *= 2;
	}

	const float BloomIntensity = 1.f;
	// Combine result
	RT_Combine = FRenderTarget::Create(ViewWidth, ViewHeight);
	RT_Combine->AddColorBuffer(EPF_RGBA8, ERTC_COLOR0);
	RT_Combine->Compile();
	UB_Combine = ti_new FSSSBloomUniformBuffer;
	UB_Combine->UniformBufferData.BloomParam.X = Exposure;
	UB_Combine->UniformBufferData.BloomParam.Y = BloomIntensity;
	UB_Combine->InitUniformBuffer();
	TT_Combine = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(3);
	TT_Combine->PutTextureInTable(TextureBlurX, 0);
	TT_Combine->PutTextureInTable(BloomPass[0][1].RT->GetColorBuffer(ERTC_COLOR0).Texture, 1);
	TT_Combine->PutTextureInTable(BloomPass[1][1].RT->GetColorBuffer(ERTC_COLOR0).Texture, 2);

	// Output result
	TT_Result = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(1);
	TT_Result->PutTextureInTable(RT_Combine->GetColorBuffer(ERTC_COLOR0).Texture, 0);
}

void FSSSSRenderer::Render(FRHI* RHI, FScene* Scene)
{
	PrepareViewUniforms(Scene);

	// Render Base Pass
	RHI->PushRenderTarget(RT_BasePass);

	RHI->SetStencilRef(1);

	const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList();
	for (const auto& Primitive : Primitives)
	{
		for (int32 m = 0; m < (int32)Primitive->MeshBuffers.size(); ++m)
		{
			FMeshBufferPtr MB = Primitive->MeshBuffers[m];
			FPipelinePtr PL = Primitive->Pipelines[m];
			FUniformBufferPtr UB = Primitive->Uniforms[m];
			FRenderResourceTablePtr TextureTable = Primitive->TextureTables[m];
			
			{
				RHI->SetMeshBuffer(MB);
				RHI->SetPipeline(PL);
				RHI->SetUniformBuffer(0, ViewUniformBuffer->UniformBuffer);
				RHI->SetUniformBuffer(1, Primitive->LightBindingUniformBuffer->UniformBuffer);

				Scene->GetSceneLights()->BindSceneLightsUniformBuffer(RHI, 2);

				if (TextureTable != nullptr)
				{
					RHI->SetRenderResourceTable(3, TextureTable);
				}
				RHI->DrawPrimitiveIndexedInstanced(MB->GetIndicesCount(), 1, 0, 0, 0);
			}
		}
	}

	RHI->PopRenderTarget();

	// Go SSS Blur Pass
	{
		RHI->PushRenderTarget(RT_SSSBlurX);
		RHI->SetPipeline(PL_SSSBlur);
		RHI->SetUniformBuffer(0, UB_SSSBlurX->UniformBuffer);
		RHI->SetUniformBuffer(1, UB_Kernel->UniformBuffer);
		RHI->SetRenderResourceTable(2, TT_SSSBlurX);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}
	{
		RHI->PushRenderTarget(RT_SSSBlurY);
		//RHI->SetPipeline(M_SSSBlur->PipelineResource);
		RHI->SetUniformBuffer(0, UB_SSSBlurY->UniformBuffer);
		RHI->SetUniformBuffer(1, UB_Kernel->UniformBuffer);
		RHI->SetRenderResourceTable(2, TT_SSSBlurY);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}

	// Add Specular
	{
		RHI->PushRenderTarget(RT_SSSBlurX);
		RHI->SetPipeline(PL_AddSpecular);
		RHI->SetRenderResourceTable(0, TT_SSSBlurX);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}

	// Bloom
	{
		// Glare detection
		RHI->PushRenderTarget(RT_GlareDetection);
		RHI->SetPipeline(PL_GlareDetection);
		RHI->SetUniformBuffer(0, UB_GlareParam->UniformBuffer);
		RHI->SetRenderResourceTable(1, TT_GlareSource);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();

		// Bloom blur X & Y
		for (int32 p = 0; p < BloomPasses; ++p)
		{
			// Pass Horizontal
			FBloomPass& PassX = BloomPass[p][0];
			RHI->PushRenderTarget(PassX.RT);
			RHI->SetPipeline(PL_Bloom);
			RHI->SetUniformBuffer(0, PassX.UB->UniformBuffer);
			RHI->SetRenderResourceTable(1, PassX.TT);
			FSRender.DrawFullScreenQuad(RHI);
			RHI->PopRenderTarget();
			
			// Pass Vertical
			FBloomPass& PassY = BloomPass[p][1];
			RHI->PushRenderTarget(PassY.RT);
			RHI->SetPipeline(PL_Bloom);
			RHI->SetUniformBuffer(0, PassY.UB->UniformBuffer);
			RHI->SetRenderResourceTable(1, PassY.TT);
			FSRender.DrawFullScreenQuad(RHI);
			RHI->PopRenderTarget();
		}
	}

	// Combine
	{
		RHI->PushRenderTarget(RT_Combine);
		RHI->SetPipeline(PL_Combine);
		RHI->SetUniformBuffer(0, UB_Combine->UniformBuffer);
		RHI->SetRenderResourceTable(1, TT_Combine);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}

	FSRender.DrawFullScreenTexture(RHI, TT_Result);
}
