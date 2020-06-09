/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TThreadIO : public TTaskThread
	{
	public:
		TThreadIO();
		virtual ~TThreadIO();

		virtual void OnThreadStart() override;

		inline static bool IsIOThread()
		{
			return TThread::GetThreadId() == IOThreadId;
		}

	protected:

	protected:
		static TThreadId IOThreadId;
	};

	inline bool IsIOThread()
	{
		return TThreadIO::IsIOThread();
	}
}
