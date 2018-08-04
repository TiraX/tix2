/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRHI;
	class FScene;

	// Renderer interface
	class TI_API FRenderer
	{
	public: 
		FRenderer();
		virtual ~FRenderer();

		virtual void PrepareViewUniforms();
		virtual void Render(FRHI* RHI, FScene* Scene);

	protected:
		virtual void DrawMeshBuffer(FMeshBufferPtr InMeshBuffer, FPipelinePtr InPipeline);
	};
}
