/*
	TiX Engine v2.0 Copyright (C) 2018
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
			text.erase(0, text.find_first_not_of(_T(" \n\r\t")));
			text.erase(text.find_last_not_of(_T(" \n\r\t")) + 1);
		}
		return text;
	}

	inline TVector<float> ReadFloatArray(const char* float_array_str, const char seperator = ' ')
	{
		TVector<float> Result;

		TString value = float_array_str;
		value = trim(value);

		int str_len = (int)strlen(value.c_str()) + 1;
		char* str_cpy = new char[str_len];
		strcpy_s(str_cpy, str_len, value.c_str());
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
		char* str_cpy = new char[str_len];
		strcpy_s(str_cpy, str_len, value.c_str());
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
		char* str_cpy = new char[str_len];
		strcpy_s(str_cpy, str_len, value.c_str());
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

	/////////////////////////////////////////////////////////////////
	struct TResMeshSegment
	{
		float* Data;
		int32 StrideInFloat;
	};

	struct TMeshDefine
	{
		TMeshDefine(const TString& InName, int32 InNumVertices, int32 InNumTriangles)
			: Name(InName)
			, NumVertices(InNumVertices)
			, NumTriangles(InNumTriangles)
		{
			memset(Segments, 0, sizeof(TResMeshSegment*) * ESSI_TOTAL);
		}
		~TMeshDefine()
		{
			for (int32 i = 0; i < ESSI_TOTAL; ++i)
			{
				SAFE_DELETE(Segments[i]);
			}
		}

		TString Name;
		int32 NumVertices;
		int32 NumTriangles;
		TResMeshSegment* Segments[ESSI_TOTAL];

		void AddSegment(E_MESH_STREAM_INDEX InStreamType, float* InData, int32 InStrideInByte);
	};

	class TResMeshHelper
	{
	public:
		TResMeshHelper();
		~TResMeshHelper();

		static bool LoadObjFile(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings);

		TMeshDefine& AddMesh(const TString& Name, int32 NumVertices, int32 NumTriangles);
		void OutputMesh(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		TVector<TMeshDefine> Meshes;
	};
}