/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_VAR_TYPE
	{
		VAR_INT,
		VAR_FLOAT,
		VAR_STRING,
	};

	struct TI_API TVarValue
	{
		union
		{
			int32 VInt;
			float VFloat;
		};
		TString VString;
		int32 VType;

		TVarValue()
			: VInt(0)
			, VType(VAR_INT)
		{}

		TVarValue(const TVarValue& Other)
		{
			VFloat = Other.VFloat;
			VString = Other.VString;
			VType = Other.VType;
		}
	};

	// THMap< Group, THMap<Key, Value> >
	typedef THMap<TString, THMap<TString, TVarValue> > TINIData;

	class TI_API TINIParser
	{
	public:
		TINIParser(const TString& IniFilename);
		~TINIParser();

		bool Parse(TINIData& OutData);
	private:
		enum {
			LINE_GROUP,
			LINE_KEYVALUE,
		};
		int32 ParseLine(const TString& Line, TString& Group, TString& Key, TString& Value);
		TString FileName;
	};
}
