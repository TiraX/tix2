/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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

		virtual void InitInRenderThread() = 0;
		virtual void InitRenderFrame(FScene* Scene) = 0;
		virtual void EndRenderFrame(FScene* Scene) = 0;
		virtual void Render(FRHI* RHI, FScene* Scene) = 0;

		virtual FUniformBufferPtr GetCounterResetUniformBuffer() = 0;
	protected:
	};
}
