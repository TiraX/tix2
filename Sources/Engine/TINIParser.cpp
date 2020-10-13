/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TINIParser.h"
#include "ini.h"

namespace tix
{
	inline bool IsNum(int8 C)
	{
		return C >= '0' && C <= '9';
	}

	inline TVarValue ParseValue(const TString& VString)
	{
		int32 VType = VAR_INT;
		const int8* P = VString.c_str();
		// Check value type
		for (int32 i = 0; i < VString.size(); ++i)
		{
			if (VType == VAR_INT && *P == '.')
			{
				VType = VAR_FLOAT;
			}
			else if (!IsNum(*P))
			{
				VType = VAR_STRING;
				break;
			}
			++P;
		}
		TVarValue V;
		V.VType = VType;
		switch (VType)
		{
		case VAR_INT:
			V.VInt = atoi(VString.c_str());
			break;
		case VAR_FLOAT:
			V.VFloat = (float)atof(VString.c_str());
			break;
		default:
			V.VString = VString;
			break;
		}
		return V;
	}
	
	TString TINIParser::MakeKey(const TString& section, const TString& name)
	{
		TString key = section + "." + name;
		// Convert to lower case to make section/name lookups case-insensitive
		//std::transform(key.begin(), key.end(), key.begin(), ::tolower);
		return key;
	}

	static int32 ValueHandler(void* user, const int8* section, const int8* name, const int8* value)
	{
		if (!name)  // Happens when INI_CALL_HANDLER_ON_NEW_SECTION enabled
			return 1;
		TINIData* Data = static_cast<TINIData*>(user);
		TString key = TINIParser::MakeKey(section, name);
		(*Data)[key] = ParseValue(value ? value : "");
		return 1;
	}

	TINIParser::TINIParser(const TString& IniFilename)
		: FileName(IniFilename)
	{
	}

	TINIParser::~TINIParser()
	{
	}


	bool TINIParser::Parse(TINIData& OutData)
	{
		int _error = ini_parse(FileName.c_str(), ValueHandler, &OutData);
		return _error == 0;
	}
}