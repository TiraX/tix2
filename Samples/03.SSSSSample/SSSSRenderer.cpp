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
	FRenderer::InitInRenderThread();

	// Render target test case
	RTTest = FRenderTarget::Create(1280, 720);
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

			DrawMeshBuffer(RHI, MB, PL, ViewUniformBuffer->UniformBuffer);
		}
	}
	
	RHI->PopRenderTarget();

	DrawFullScreenTexture(RHI, RTTest->GetColorBuffer(ERTC_COLOR0).Texture);
}