/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderer.h"

namespace tix
{
	FRenderer::FRenderer()
	{
	}

	FRenderer::~FRenderer()
	{
	}

	void FRenderer::PrepareViewUniforms()
	{
		TI_ASSERT(0);
	}

	void FRenderer::Render(FRHI* RHI, FScene* Scene)
	{
		Scene->CollectAllMeshRelevance();

		PrepareViewUniforms();

		const TVector<FMeshRelevance>& Meshes = Scene->GetStaticDrawList();
		for (const auto& Relevance : Meshes)
		{
			DrawMeshBuffer(Relevance.MeshBuffer, Relevance.Pipeline);
		}
	}

	void FRenderer::DrawMeshBuffer(FMeshBufferPtr InMeshBuffer, FPipelinePtr Pipeline)
	{
		TI_ASSERT(0);
	}
}