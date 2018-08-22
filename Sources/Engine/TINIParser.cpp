/*
	TiX Engine v2.0 Copyright (C) 2018
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

	inline TString& Trim(TString& text)
	{
		if (!text.empty())
		{
			text.erase(0, text.find_first_not_of(" \r"));
			text.erase(text.find_last_not_of(" \r") + 1);
		}
		return text;
	}

	bool TINIParser::Parse()
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
				Trim(Line);

				if (!Line.empty())
				{
					TString Key, Value;
					ParseLine(Line, CurrentGroup, Key, Value);
				}

				++Cursor;
				Start = Cursor;
			}

			// Parse Left things
			TString Line = Start;
			Trim(Line);

			if (!Line.empty())
			{
				TString Key, Value;
				ParseLine(Line, CurrentGroup, Key, Value);
			}

			ti_delete[] FileBuffer;

			return true;
		}
		return false;
	}

	void TINIParser::ParseLine(const TString& Line, TString& Group, TString& Key, TString& Value)
	{
		// Find key and value
		if (Line[0] == '[')
		{
			Group = Line.substr(1, Line.size() - 2);
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
				Trim(Key);
				Trim(Value);
			}
			else
			{
				Key = Line;
				Trim(Key);
			}
		}
	}
}