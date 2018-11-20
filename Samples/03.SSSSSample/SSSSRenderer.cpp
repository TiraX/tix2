/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SSSSRenderer.h"

FSSSSRenderer::FSSSSRenderer()
{
}

FSSSSRenderer::~FSSSSRenderer()
{
	RTTextureTable = nullptr;
	RTTest = nullptr;
}

void FSSSSRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	// Render target test case
	RTTest = FRenderTarget::Create(1280, 720);
#if defined (TIX_DEBUG)
	RTTest->SetResourceName("RTTest");
#endif
	RTTest->AddColorBuffer(EPF_BGRA8, ERTC_COLOR0);
	RTTest->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8);
	RTTest->Compile();

	RTTextureTable = FRHI::Get()->GetRenderResourceHeap(EHT_TEXTURE).AllocateTable(1);
	RTTextureTable->PutTextureInTable(RTTest->GetColorBuffer(ERTC_COLOR0).Texture, 0);
}

void FSSSSRenderer::Render(FRHI* RHI, FScene* Scene)
{
	PrepareViewUniforms(Scene);

	RHI->PushRenderTarget(RTTest);

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

	FSRender.DrawFullScreenTexture(RHI, RTTextureTable);
}