/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TINIParser.h"

namespace tix
{
	TINIParser::TINIParser(const TString& IniFilename)
		: FileName(IniFilename)
	{
	}

	TINIParser::~TINIParser()
	{
	}

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

	bool TINIParser::Parse(TINIData& OutData)
	{
		TFile IniFile;
		if (IniFile.Open(FileName, EFA_READ))
		{
			// Read file
			int8* FileBuffer = ti_new int8[IniFile.GetSize() + 1];
			IniFile.Read(FileBuffer, IniFile.GetSize(), IniFile.GetSize());
			FileBuffer[IniFile.GetSize()] = 0;

			TString CurrentGroup = "DefaultGroup";

			int8* Start = FileBuffer;
			int8* Cursor = FileBuffer;
			// Read line
			while (*Cursor ++)
			{
				if (*Cursor != '\n')
					continue;

				*Cursor = '\0';
				TString Line = Start;
				TStringTrim(Line);

				if (!Line.empty())
				{
					TString Key, Value;
					int32 LineResult = ParseLine(Line, CurrentGroup, Key, Value);
					if (LineResult == LINE_KEYVALUE)
					{
						THMap<TString, TVarValue>& Values = OutData[CurrentGroup];
						Values[Key] = ParseValue(Value);
					}
				}

				++Cursor;
				Start = Cursor;
			}

			// Parse Left things
			TString Line = Start;
			TStringTrim(Line);

			if (!Line.empty())
			{
				TString Key, Value;
				int32 LineResult = ParseLine(Line, CurrentGroup, Key, Value);
				if (LineResult == LINE_KEYVALUE)
				{
					THMap<TString, TVarValue>& Values = OutData[CurrentGroup];
					Values[Key] = ParseValue(Value);
				}
			}

			ti_delete[] FileBuffer;

			return true;
		}
		return false;
	}

	int32 TINIParser::ParseLine(const TString& Line, TString& Group, TString& Key, TString& Value)
	{
		// Find key and value
		if (Line[0] == '[')
		{
			Group = Line.substr(1, Line.size() - 2);
			return LINE_GROUP;
		}
		else
		{
			uint64 Pos = Line.find('=');

			if (Pos == TString::npos)
				Pos = Line.find(' ');

			if (Pos != TString::npos)
			{
				Key = Line.substr(0, Pos);
				Value = Line.substr(Pos + 1);
				TStringTrim(Key);
				TStringTrim(Value);
			}
			else
			{
				Key = Line;
				TStringTrim(Key);
			}
			return LINE_KEYVALUE;
		}
	}
}