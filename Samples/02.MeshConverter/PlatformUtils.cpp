/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "PlatformUtils.h"
#if defined (TI_PLATFORM_IOS)
#include <mach-o/dyld.h>
#include <unistd.h>
#endif

namespace tix
{
	TString GetExecutablePath()
	{
		TString Ret;
		int8 Path[512];
#if defined (TI_PLATFORM_WIN32)
		::GetModuleFileName(NULL, Path, 512);
		Ret = Path;
		TStringReplace(Ret, "\\", "/");
#elif defined (TI_PLATFORM_IOS)
		uint32 BufferSize = 512;
		_NSGetExecutablePath(Path, &BufferSize);
		TI_ASSERT(BufferSize <= 512);
		Ret = Path;
		if (Ret.find("/Binary") == TString::npos && Ret.find("tix2/") == TString::npos)
		{
			Ret = getcwd(NULL, 0);
		}
#endif

		Ret = Ret.substr(0, Ret.rfind('/'));

		// if dir is not in Binary/ then find from root "tix2"
		if (Ret.find("/Binary") == TString::npos)
		{
			TString::size_type root_pos = Ret.find("tix2/");
			TI_ASSERT(root_pos != TString::npos);
			Ret = Ret.substr(0, root_pos + 5) + "Binary/";
#if defined (TI_PLATFORM_WIN32)
			Ret += "Windows";
#elif defined (TI_PLATFORM_IOS)
			Ret += "Mac";
#endif
		}

		return Ret;
	}

	int32 DeleteTempFile(TString FileName)
	{
		TString CommandLine;
#if defined (TI_PLATFORM_WIN32)
		CommandLine = "del ";
		TStringReplace(FileName, "/", "\\");
#elif defined (TI_PLATFORM_IOS)
		CommandLine = "rm ";
#endif
		CommandLine += FileName;
		return system(CommandLine.c_str());
	}
	
	bool IsDirectoryExists(const TString& Path)
	{
#if defined (TI_PLATFORM_WIN32)
		DWORD dwAttrib = GetFileAttributes(Path.c_str());

		return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
			(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
#else
		TI_ASSERT(0);
#endif
	}

	void MakeDirectory(const TString& Dir)
	{
#if defined (TI_PLATFORM_WIN32)
		if (!CreateDirectory(Dir.c_str(), nullptr))
		{
			_LOG(Error, "Failed to create directory : %s.\n", Dir.c_str());
		}
#else
		TI_ASSERT(0);
#endif
	}

	void TryMakeDirectory(const TString& InTargetPath)
	{
		TString TargetPath = InTargetPath;
		TVector<TString> Dirs;
		TString::size_type SplitIndex;
		SplitIndex = TargetPath.find('/');
		while (SplitIndex != TString::npos)
		{
			TString Dir = TargetPath.substr(0, SplitIndex);
			Dirs.push_back(Dir);
			TargetPath = TargetPath.substr(SplitIndex + 1);
			SplitIndex = TargetPath.find('/');
		}
		if (!TargetPath.empty())
		{
			Dirs.push_back(TargetPath);
		}

		TString TargetDir = "";
		for (int32 Dir = 0; Dir < (int32)Dirs.size(); ++Dir)
		{
			TargetDir += Dirs[Dir] + "/";
			if (!IsDirectoryExists(TargetDir))
			{
				MakeDirectory(TargetDir);
			}
		}
	}

	bool CreateDirectoryIfNotExist(const TString& Path)
	{
		// Directory Exists? 
		if (!IsDirectoryExists(Path))
		{
			TryMakeDirectory(Path);
		}

		if (!IsDirectoryExists(Path)) 
		{
			return false;
		}
		return true;
	}

	int32 GetProcessorCount()
	{
#if defined (TI_PLATFORM_WIN32)
		// Figure out how many cores there are on this machine
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
#else
		TI_ASSERT(0);
#endif
	}
}
