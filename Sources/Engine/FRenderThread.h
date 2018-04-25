/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderer;
	class FRHI;

	class FRenderThread : public TTaskThread
	{
	public: 
		FRenderThread();
		virtual ~FRenderThread();

		virtual void Run() override;
		virtual void OnThreadStart() override;
		virtual void OnThreadEnd() override;

	protected:
		void CreateRenderComponents();
		void DestroyRenderComponents();

	protected:
		FRHI * RHI;

		uint64 LastFrameTime;
	};
}
