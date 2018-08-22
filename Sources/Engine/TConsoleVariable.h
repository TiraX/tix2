/*
	TiX Engine v2.0 Copyright (C) 2018
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
	};

	struct TVarMapping
	{
		union
		{
			int32* VarInt;
			float* VarFloat;
		};
		TString* VarString;
		int32 VType;

		TVarMapping()
			: VarInt(nullptr)
			, VarString(nullptr)
			, VType(VAR_INT)
		{}

		TVarMapping(const TVarMapping& Other)
		{
			VarInt = Other.VarInt;
			VarString = Other.VarString;
			VType = Other.VType;
		}
	};

	class TI_API TCVar
	{
	public:
		TCVar(const TString& VarName, int32& VarInt);
		TCVar(const TString& VarName, float& VarFloat);
		TCVar(const TString& VarName, TString& VarString);
	};

	class TConsoleVariables
	{
	public:
		TI_API static TConsoleVariables* Get();
		TI_API static void Init();
		TI_API static void Destroy();

	private:
		TConsoleVariables();
		~TConsoleVariables();
		static TConsoleVariables* s_cvar_instance;
		static void AddCVar(const TString& VarName, int32* VarInt)
		{
			TVarMapping M;
			M.VarInt = VarInt;
			M.VType = VAR_INT;
			VarMap[VarName] = M;
		}
		static void AddCVar(const TString& VarName, float* VarFloat)
		{
			TVarMapping M;
			M.VarFloat = VarFloat;
			M.VType = VAR_FLOAT;
			VarMap[VarName] = M;
		}
		static void AddCVar(const TString& VarName, TString* VarString)
		{
			TVarMapping M;
			M.VarString = VarString;
			M.VType = VAR_STRING;
			VarMap[VarName] = M;
		}

		void InitFromIni();
	private:
		static THMap<TString, TVarMapping> VarMap;
		friend class TCVar;
	};
}
