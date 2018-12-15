/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SSSSRenderer.h"
#include "SeparableSSS.h"

FSSSSRenderer::FSSSSRenderer()
{
	S4Effect = ti_new SeparableSSS(1600, 900, DEG_TO_RAD(40), 250.f);
	TI_TODO("Try FoV 20.");
	const TString SSSBlurMaterialName = "M_SSSBlur.tres";
	M_SSSBlur = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(SSSBlurMaterialName).get());
}

FSSSSRenderer::~FSSSSRenderer()
{
	ti_delete S4Effect;
	M_SSSBlur = nullptr;
	RT_BasePass = nullptr;
	RT_SSSBlurX = nullptr;
	RT_SSSBlurY = nullptr;
	TT_SSSBlurX = nullptr;
	TT_SSSBlurY = nullptr;
	UB_SSSBlurX = nullptr;
	UB_SSSBlurY = nullptr;
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
	
	// Setup SSS blur pass render targets and texture tables
	// RT BlurX
	RT_SSSBlurX = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_SSSBlurX->SetResourceName("SSSBlurX");
#endif
	RT_SSSBlurX->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0);
	RT_SSSBlurX->Compile();

	// TT BlurX
	FTexturePtr SceneColor = RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture;
	FTexturePtr SceneDepth = RT_BasePass->GetDepthStencilBuffer().Texture;

	TT_SSSBlurX = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(2);
	TT_SSSBlurX->PutTextureInTable(SceneColor, 0);
	TT_SSSBlurX->PutTextureInTable(SceneDepth, 1);

	// RT BlurY
	RT_SSSBlurY = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_SSSBlurY->SetResourceName("SSSBlurY");
#endif
	RT_SSSBlurY->AddColorBuffer(SceneColor, ERTC_COLOR0);
	RT_SSSBlurY->Compile();

	// TT BlurY
	FTexturePtr TextureBlurX = RT_SSSBlurX->GetColorBuffer(ERTC_COLOR0).Texture;
	TT_SSSBlurY = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(2);
	TT_SSSBlurY->PutTextureInTable(TextureBlurX, 0);
	TT_SSSBlurY->PutTextureInTable(SceneDepth, 1);

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
}

void FSSSSRenderer::Render(FRHI* RHI, FScene* Scene)
{
	PrepareViewUniforms(Scene);

	// Render Base Pass
	RHI->PushRenderTarget(RT_BasePass);

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
		RHI->SetPipeline(M_SSSBlur->PipelineResource);
		RHI->SetUniformBuffer(0, UB_SSSBlurX->UniformBuffer);
		RHI->SetRenderResourceTable(1, TT_SSSBlurX);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}
	{
		RHI->PushRenderTarget(RT_SSSBlurY);
		//RHI->SetPipeline(M_SSSBlur->PipelineResource);
		RHI->SetUniformBuffer(0, UB_SSSBlurY->UniformBuffer);
		RHI->SetRenderResourceTable(1, TT_SSSBlurY);
		FSRender.DrawFullScreenQuad(RHI);
		RHI->PopRenderTarget();
	}

	FSRender.DrawFullScreenTexture(RHI, TT_SSSBlurX);
}