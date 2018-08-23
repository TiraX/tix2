/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	inline void TStringTrim(TString& String)
	{
		if (!String.empty())
		{
			String.erase(0, String.find_first_not_of(" \r"));
			String.erase(String.find_last_not_of(" \r") + 1);
		}
	}

	inline void TStringReplace(TString &String, const TString& StringSrc, const TString& StringDest)
	{
		TString::size_type Pos = 0;
		TString::size_type SrcLen = StringSrc.size();
		TString::size_type DesLen = StringDest.size();
		Pos = String.find(StringSrc, Pos);
		while ((Pos != string::npos))
		{
			String.replace(Pos, SrcLen, StringDest);
			Pos = String.find(StringSrc, (Pos + DesLen));
		}
	}
}
