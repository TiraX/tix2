/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TThreadIO;
	class TThreadLoading : public TTaskThread
	{
	public:
		static void CreateLoadingThread();
		static void DestroyLoadingThread();

	private:
		static TThreadLoading* LoadingThread;
		TThreadLoading();
		virtual ~TThreadLoading();

		virtual void Start();
		virtual void Stop();

	protected:

	protected:
		TThreadIO * IOThread;
	};
}
