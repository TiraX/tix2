/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include <stdarg.h>
#include "TLog.h"

#ifdef TI_PLATFORM_ANDROID
#include <android/log.h>
#endif

namespace tix
{
	TLog::TLog()
	{
		LogFilename = "txengine.log";
	}

	TLog::~TLog()
	{}

	void TLog::DoLog(E_LOG_LEVEL LogLevel, const char* Format, ...)
	{
#ifndef TI_PLATFORM_ANDROID
#	ifdef TI_PLATFORM_WIN32
		char *tmp	= ti_new char[65536];
#	else
		char tmp[1024];
#	endif
		va_list marker;
		va_start(marker, Format);
#	ifdef TI_PLATFORM_WIN32
		vsprintf_s(tmp, 65536, Format, marker);
#	else
		vsprintf(tmp, Format, marker);
#	endif
		va_end(marker);

#	ifdef TI_PLATFORM_WIN32
		if (LogLevel == Error || LogLevel == Fatal)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_RED);
		}
		else if (LogLevel == Warning)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN);
		}
		else
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
#	endif

		printf("%s", tmp);

#	ifdef TI_PLATFORM_WIN32
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
		ti_delete[] tmp;
#	endif
#else
		char buf[1024];

		va_list marker;
		va_start(marker, format);
		vsnprintf(buf, 1024-3, format, marker);
		strcat(buf, "\n");
		va_end(marker);

		__android_log_print(ANDROID_LOG_DEBUG, "tix debug",  "%s", buf);
#endif
		if (LogLevel == Fatal)
		{
			TI_ASSERT(0);
		}
	}
}
