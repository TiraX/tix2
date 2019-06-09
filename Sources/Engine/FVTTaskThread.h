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
		virtual void Run() override;

	protected:
		void AnalysisBuffer();

	private:
		TMutex BufferMutex;
		TList<TStreamPtr> Buffers;

		TVector<int32> VTLoadTasks;
	};

}