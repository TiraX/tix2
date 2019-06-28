/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FVTTaskThread : public TThread
	{
	public:
		FVTTaskThread();
		virtual ~FVTTaskThread();

		TI_API void AddUVBuffer(TStreamPtr Buffer);
		virtual void OnThreadStart() override;
		virtual void Run() override;

	protected:
		void AnalysisBuffer();
		void DoLoadingTask();
		void OutputDebugTasks();

	private:
		TMutex BufferMutex;

		// VT UV Buffers of 1/8 x 1/8 screen, send from Render Thread
		TList<TStreamPtr> Buffers;

		// Task execute order, point to VTLoadTasks
		TList<uint32> VTTaskOrder;

		// The actual load task, Key is Page Index, Value is Page Load Info
		THMap<uint32, FVTSystem::FPageInfo> VTLoadTasks;

		// Cached textures, Key is Page Index, Value is FTexture
		THMap<uint32, FTexturePtr> CachedTextures;
	};

}