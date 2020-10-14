/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TPlatformUtils
	{
	public:
		TI_API static TString GetExecutablePath();
		TI_API static int32 DeleteTempFile(TString FileName);
		TI_API static int32 OverwriteFile(TString SrcName, TString DstName);
		TI_API static bool CreateDirectoryIfNotExist(const TString& Path);
		TI_API static int32 GetProcessorCount();
	};
}
