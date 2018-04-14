/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TLog
	{
	public:
		TLog();
		virtual ~TLog();

		TI_API static void Log(const char* format, ...);
	private:
		TString	LogFilename;
	};
}

#ifdef TIX_DEBUG
#	define _LOG(format, ...)	TLog::Log(format, ##__VA_ARGS__)
#else
#	define _LOG(format, ...)	TLog::Log(format, ##__VA_ARGS__)
#endif

#define _DLOG	_LOG