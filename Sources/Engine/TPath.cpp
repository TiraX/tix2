/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TPath.h"

namespace tix
{
	TString TPath::WorkPath;
	TCVar CVarWorkPath("WorkPath", TPath::WorkPath, 0);

	TString TPath::GetAbsolutePath(const TString& RelativePath)
	{
		TI_ASSERT(!WorkPath.empty());
		if (WorkPath.at(WorkPath.size() - 1) != '/')
		{
			// Make path format correct.
			TStringReplace(WorkPath, "\\", "/");
			if (WorkPath.at(WorkPath.size() - 1) != '/')
			{
				WorkPath += '/';
			}
		}
		return WorkPath + RelativePath;
	}
}