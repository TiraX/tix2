/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{

	// help functions
	inline TString& trim(TString& text)
	{
		if (!text.empty())
		{
			text.erase(0, text.find_first_not_of(" \n\r\t"));
			text.erase(text.find_last_not_of(" \n\r\t") + 1);
		}
		return text;
	}

	inline TString& tolower(TString& text)
	{
		transform(text.begin(), text.end(), text.begin(), ::tolower);
		return text;
	}

	inline TVector<float> ReadFloatArray(const char* float_array_str, const char seperator = ' ')
	{
		TVector<float> Result;

		TString value = float_array_str;
		value = trim(value);

		int str_len = (int)strlen(value.c_str()) + 1;
		char* str_cpy = ti_new char[str_len];
		strcpy(str_cpy, value.c_str());
		char *start, *end;
		start = str_cpy;
		end = start;

		float num;
		while (*end)
		{
			if (*end == seperator)
			{
				*end = 0;
				num = (float)atof(start);
				start = end + 1;
				Result.push_back(num);
			}
			++end;
		}

		if (strlen(start) > 0)
		{
			num = (float)atof(start);
			Result.push_back(num);
		}

		delete[] str_cpy;
		return Result;
	}

	inline TVector<int> ReadIntArray(const char* int_array_str, const char seperator = ' ')
	{
		TVector<int> Result;

		TString value = int_array_str;
		value = trim(value);

		int str_len = (int)strlen(value.c_str()) + 1;
		char* str_cpy = ti_new char[str_len];
		strcpy(str_cpy, value.c_str());
		char *start, *end;
		start = str_cpy;
		end = start;

		int num;
		while (*end)
		{
			if (*end == seperator)
			{
				*end = 0;
				num = atoi(start);
				start = end + 1;
				Result.push_back(num);
			}
			++end;
		}

		if (strlen(start) > 0)
		{
			num = atoi(start);
			Result.push_back(num);
		}

		delete[] str_cpy;
		return Result;
	}

	inline TVector<TString> ReadStringArray(const char* string_array_str, const char sep = ' ')
	{
		TVector<TString> Result;

		TString value = string_array_str;
		value = trim(value);

		int str_len = (int)strlen(value.c_str()) + 1;
		char* str_cpy = ti_new char[str_len];
		strcpy(str_cpy, value.c_str());
		char *start, *end;
		start = str_cpy;
		end = start;

		TString str;
		while (*end)
		{
			if (*end == sep)
			{
				*end = 0;
				str = start;
				start = end + 1;
				Result.push_back(str);
			}
			++end;
		}

		if (strlen(start) > 0)
		{
			str = start;
			Result.push_back(str);
		}

		delete[] str_cpy;
		return Result;
	}

	inline void FillZero4(TStream& Stream)
	{
		char zero[64] = { 0 };
		int32 bytes = TMath::Align4(Stream.GetLength()) - Stream.GetLength();
		TI_ASSERT(bytes <= 64);
		Stream.Put(zero, bytes);
	}

	inline int32 AddStringToList(TVector<TString>& Strings, const TString& String)
	{
		for (int32 s = 0 ; s < (int32)Strings.size() ; ++ s)
		{
			if (Strings[s] == String)
			{
				return s;
			}
		}
		Strings.push_back(String);
		return (int32)Strings.size() - 1;
	}

	inline void SaveStringList(const TVector<TString>& Strings, TStream& Stream)
	{
		char zero[8] = { 0 };

		int32* string_offsets = ti_new int32[Strings.size()];
		int32 offset = 0;
		for (int32 i = 0; i < (int32)Strings.size(); ++i)
		{
			const TString& s = Strings[i];
			offset += ((s.size() + 4) & ~3);
			string_offsets[i] = offset;
		}

		Stream.Put(string_offsets, (int32)Strings.size() * sizeof(int32));
		FillZero4(Stream);
		for (int i = 0; i < (int)Strings.size(); ++i)
		{
			const TString& s = Strings[i];
			int32 len = (((int32)s.size() + 4) & ~3);
			int32 real_len = (int32)s.size();
			Stream.Put(s.c_str(), real_len);
			Stream.Put(zero, len - real_len);;
		}
		ti_delete[] string_offsets;
	}

	inline uint8 FloatToUNorm(float n)
	{
		if (n < -1.f)
			n = -1.f;
		if (n > 1.f)
			n = 1.f;
		n = n * 0.5f + 0.5f;
		float n0 = n * 255.f + 0.5f;
		return (uint8)n0;
	}
	inline uint8 FloatToColor(float n)
	{
		if (n < 0.f)
			n = 0.f;
		if (n > 1.f)
			n = 1.f;
		float n0 = n * 255.f + 0.5f;
		return (uint8)n0;
	}
	/////////////////////////////////////////////////////////////////
	class TResFileHelper
	{
	public:
		TResFileHelper();
		~TResFileHelper();

		TStream& GetChunk(E_CHUNK_LIB ChunkType);
		bool SaveFile(const TString& Filename);

		TVector<TString> Strings;
		TStream ChunkStreams[ECL_COUNT];
	};
}
