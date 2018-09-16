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
			FTexturePtr Tex = nullptr;
			if (Primitive->Textures.size() > 0)
			{
				Tex = Primitive->Textures[0];
			}

			{
				RHI->SetMeshBuffer(MB);
				RHI->SetPipeline(PL);
				RHI->SetUniformBuffer(0, ViewUniformBuffer->UniformBuffer);

				if (Tex != nullptr)
					RHI->SetShaderTexture(1, Tex);
				RHI->DrawPrimitiveIndexedInstanced(MB->GetIndicesCount(), 1, 0, 0, 0);
			}
		}
	}
	
	RHI->PopRenderTarget();

	FSRender.DrawFullScreenTexture(RHI, RTTest->GetColorBuffer(ERTC_COLOR0).Texture);
}