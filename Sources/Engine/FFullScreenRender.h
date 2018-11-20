/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRHI;

	// Renderer interface
	class TI_API FFullScreenRender
	{
	public: 
		FFullScreenRender();
		~FFullScreenRender();

		void InitCommonResources(FRHI* RHI);
		void DrawFullScreenTexture(FRHI* RHI, FTexturePtr Texture);
		void DrawFullScreenTexture(FRHI* RHI, FRenderResourceTablePtr TextureTable);

	protected:
		bool bInited;
		// Common Resources
		struct FullScreenVertex
		{
			vector3df Position;
			vector2df UV;
		};
		FMeshBufferPtr FullScreenQuad;
		FShaderBindingPtr FullScreenBinding;
		FPipelinePtr FullScreenPipeline;
	};
}
