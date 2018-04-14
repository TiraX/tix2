/*
	TiX Engine v2.0 Copyright (C) 2018
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

	void TLog::Log(const char* format, ...)
	{
#ifndef TI_PLATFORM_ANDROID
#	ifdef TI_PLATFORM_WIN32
		char *tmp	= ti_new char[65536];
#	else
		char tmp[1024];
#	endif
		va_list marker;
		va_start(marker, format);
#	ifdef TI_PLATFORM_WIN32
		vsprintf_s(tmp, 65536, format, marker);
#	else
		vsprintf_s(tmp, 1024, format, marker);
#	endif
		va_end(marker);

#	ifdef TI_PLATFORM_WIN32
		const char* err_str	= strstr(tmp, "Error");
		if (err_str)
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_RED);
		}
		else if (strstr(tmp, "Warning"))
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

		printf(tmp);
#	ifdef TI_PLATFORM_WIN32
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
	}
}
