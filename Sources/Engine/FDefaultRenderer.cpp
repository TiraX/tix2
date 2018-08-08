/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FDefaultRenderer.h"
#include "FUniformBufferView.h"

namespace tix
{
	FDefaultRenderer::FDefaultRenderer()
	{
	}

	FDefaultRenderer::~FDefaultRenderer()
	{
		TI_ASSERT(IsRenderThread());
		ViewUniformBuffer = nullptr;
	}

	void FDefaultRenderer::PrepareViewUniforms(FScene* Scene)
	{
		if (ViewUniformBuffer == nullptr)
			ViewUniformBuffer = ti_new FViewUniformBuffer();

		const FViewProjectionInfo& VPInfo = Scene->GetViewProjection();
		ViewUniformBuffer->UniformBufferData.ViewProjection = (VPInfo.MatProj * VPInfo.MatView).getTransposed();

		ViewUniformBuffer->InitUniformBuffer();
		TI_TODO("Do not update this every frame, only when it changed.");
	}

	void FDefaultRenderer::Render(FRHI* RHI, FScene* Scene)
	{
		Scene->CollectAllMeshRelevance();

		PrepareViewUniforms(Scene);

		const TVector<FMeshRelevance>& Meshes = Scene->GetStaticDrawList();
		for (const auto& Relevance : Meshes)
		{
			DrawMeshBuffer(RHI, Relevance.MeshBuffer, Relevance.Pipeline, ViewUniformBuffer->UniformBuffer);
		}
	}

	void FDefaultRenderer::DrawMeshBuffer(FRHI * RHI, FMeshBufferPtr InMeshBuffer, FPipelinePtr Pipeline, FUniformBufferPtr InUniformBuffer)
	{
		RHI->SetMeshBuffer(InMeshBuffer);
		RHI->SetPipeline(Pipeline);
		RHI->SetUniformBuffer(InUniformBuffer);

		RHI->DrawPrimitiveIndexedInstanced(InMeshBuffer->GetIndicesCount(), 1, 0, 0, 0);
	}
}