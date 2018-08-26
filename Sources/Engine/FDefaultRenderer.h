/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "FUniformBufferView.h"

namespace tix
{
	class FRHI;
	class FScene;

	// Default Renderer
	class TI_API FDefaultRenderer : public FRenderer
	{
	public:
		FDefaultRenderer();
		virtual ~FDefaultRenderer();

		virtual void InitInRenderThread() override;
		virtual void Render(FRHI* RHI, FScene* Scene) override;

		virtual void PrepareViewUniforms(FScene* Scene);

	protected:
		virtual void DrawMeshBuffer(FRHI * RHI, FMeshBufferPtr InMeshBuffer, FPipelinePtr InPipeline, FUniformBufferPtr InUniformBuffer);

	protected:
		FViewUniformBufferPtr ViewUniformBuffer;
	};
}
