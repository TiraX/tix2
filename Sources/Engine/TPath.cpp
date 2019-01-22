/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TPath.h"

namespace tix
{
	TString TPath::GetAbsolutePath(const TString& RelativePath)
	{
		return TEngine::Get()->GetDevice()->GetAbsolutePath() + RelativePath;
	}
}