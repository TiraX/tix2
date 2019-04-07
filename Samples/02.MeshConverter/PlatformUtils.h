/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	TString GetExecutablePath();
	int32 DeleteTempFile(TString FileName);
	bool CreateDirectoryIfNotExist(const TString& Path);
}
