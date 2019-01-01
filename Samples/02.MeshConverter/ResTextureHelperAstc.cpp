/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"

namespace tix
{
	TString GetExecutablePath()
	{
		TString Ret;
		int8 Path[512];
#if defined (TI_PLATFORM_WIN32)
		::GetModuleFileName(NULL, Path, 512);
#elif defined (TI_PLATFORM_IOS)
		int32 BufferSize = 512;
		_NSGetExecutablePath(Path, &BufferSize);
		TI_ASSERT(BufferSize <= 512);
#endif
		Ret = Path;
		TStringReplace(Ret, "\\", "/");

		Ret = Ret.substr(0, Ret.rfind('/'));

		// if dir is not in Binary/ then find from root "tix2"
		if (Ret.find("/Binary") == TString::npos)
		{
			TString::size_type root_pos = Ret.find("tix2/");
			TI_ASSERT(root_pos != TString::npos);
			Ret = Ret.substr(0, root_pos + 5) + "Binary/";
#if defined (TI_PLATFORM_WIN32)
			Ret += "Windows/";
#elif defined (TI_PLATFORM_IOS)
			Ret += "Mac/";
#endif
		}

		return Ret;
	}
	TResTextureDefine* TResTextureHelper::LoadAstcFile(const TString& Filename)
	{
		TString TextureName = Filename;
		// if src format is dds, decode it first
		if (Filename.rfind(".dds") != TString::npos)
		{
			TString TGAName = Filename.substr(0, Filename.rfind(".dds")) + ".tga";
			if (!DecodeDXT(Filename, TGAName))
			{
				printf("Error: Failed to decode dds to tga. [%s]\n", Filename.c_str());
				return nullptr;
			}
			TextureName = TGAName;
		}

		// Find ASTC converter
		TString ExePath = GetExecutablePath();
		TString ASTCConverter = ExePath + "astcenc -c ";
		ASTCConverter += TextureName + " ";
		ASTCConverter += TextureName + ".astc 6x6 -medium -silentmode";
		printf("Converting ASTC: %s\n", ASTCConverter.c_str());

		// Convert to astc
		int ret = system(ASTCConverter.c_str());

		return nullptr;
	}
}
