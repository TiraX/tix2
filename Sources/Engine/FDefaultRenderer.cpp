/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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
		if (Scene->HasSceneFlag(FScene::ViewProjectionDirty))
		{
			// Always make a new View uniform buffer for on-the-fly rendering
			ViewUniformBuffer = ti_new FViewUniformBuffer();

			const FViewProjectionInfo& VPInfo = Scene->GetViewProjection();
			ViewUniformBuffer->UniformBufferData.ViewProjection = VPInfo.MatProj * VPInfo.MatView;
			ViewUniformBuffer->UniformBufferData.ViewDir = VPInfo.CamDir;
			ViewUniformBuffer->UniformBufferData.ViewPos = VPInfo.CamPos;

			ViewUniformBuffer->InitUniformBuffer();

			// remove vp dirty flag
			Scene->SetSceneFlag(FScene::ViewProjectionDirty, false);
		}
	}

	void FDefaultRenderer::InitInRenderThread()
	{
	}

	void FDefaultRenderer::Render(FRHI* RHI, FScene* Scene)
	{
		PrepareViewUniforms(Scene);

		const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList();
		for (const auto& Primitive : Primitives)
		{
			for (int32 m = 0; m < (int32)Primitive->MeshBuffers.size(); ++m)
			{
				FMeshBufferPtr MB = Primitive->MeshBuffers[m];
				FPipelinePtr PL = Primitive->Pipelines[m];
				FUniformBufferPtr UB = Primitive->Uniforms[m];

				RHI->SetMeshBuffer(MB);
				RHI->SetPipeline(PL);
				RHI->SetUniformBuffer(0, UB);

				RHI->DrawPrimitiveIndexedInstanced(MB->GetIndicesCount(), 1, 0, 0, 0);
			}
		}
	}
}