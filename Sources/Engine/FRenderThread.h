/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderer;
	class FRHI;

	class FRenderThread : public TThread
	{
	public: 
		FRenderThread();
		~FRenderThread();

		virtual void Run() override;

	protected:
		FRenderer * Renderer;
		FRHI * RHI;
	};
}
