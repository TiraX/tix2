/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API TINIParser
	{
	public:
		TINIParser(const TString& IniFilename);
		~TINIParser();

		bool Parse();
	private:
		void ParseLine(const TString& Line, TString& Group, TString& Key, TString& Value);
		TString FileName;
	};
}
