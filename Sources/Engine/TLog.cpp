/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include <stdarg.h>
#include "TLog.h"
#include "TTimer.h"

#ifdef TI_PLATFORM_ANDROID
#include <android/log.h>
#endif

namespace tix
{
	int32 TLog::SilenceLog = 0;
	TFile* LogFile = nullptr;

	TLog::TLog()
	{
		if (LogFile == nullptr)
		{
			int32 h, m, s;
			TTimer::GetCurrentTimeSeconds(h, m, s);
			int8 LogName[128];
			sprintf(LogName, "tixlog_%02d-%02d-%02d.log", h, m, s);
			LogFilename = LogName;

			TString LogPath = "Saved/Logs/";
			TPlatformUtils::CreateDirectoryIfNotExist(LogPath);

			LogFile = ti_new TFile;
			if (!LogFile->Open(LogPath + LogFilename, EFA_CREATEWRITE))
			{
				ti_delete LogFile;
				LogFile = nullptr;
			}
		}
	}

	TLog::~TLog()
	{
		SAFE_DELETE(LogFile);
	}

	void TLog::DoLog(E_LOG_LEVEL LogLevel, const char* Format, ...)
	{
#ifndef TI_PLATFORM_ANDROID
		char *tmp	= ti_new char[65536];

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

		if (SilenceLog == 0)
			printf("%s", tmp);
		if (LogFile != nullptr)
		{
			LogFile->Write(tmp, (int32)strlen(tmp));
		}

#	ifdef TI_PLATFORM_WIN32
		{
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),
				FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		}
#	endif
		ti_delete[] tmp;
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
