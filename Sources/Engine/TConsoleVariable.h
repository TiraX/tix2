/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TINIParser.h"

namespace tix
{
	class TI_API TCVar
	{
	public:
		TCVar(const TString& VarName, int32& VarInt, uint32 InFlag);
		TCVar(const TString& VarName, float& VarFloat, uint32 InFlag);
		TCVar(const TString& VarName, TString& VarString, uint32 InFlag);

		void ValueUpdated(const TVarValue& Value);
	private:
		TString CVarName;
		int32 * ValueInt;
		float * ValueFloat;
		TString * ValueString;
		int32 ValueType;
		uint32 Flag;
	};

	class TConsoleVariables
	{
	public:
		TI_API static TConsoleVariables* Get();
		TI_API static void Init();
		TI_API static void Destroy();

		static void AddCVar(const TString& VarName, TCVar* Var)
		{
			THMap<TString, TCVar*>& VarMap = GetVarMap();
			VarMap[VarName] = Var;
		}
	private:
		TConsoleVariables();
		~TConsoleVariables();
		static TConsoleVariables* s_cvar_instance;

		static THMap<TString, TCVar*>& GetVarMap()
		{
			static THMap<TString, TCVar*> s_VarMap;
			return s_VarMap;
		}

		void InitFromIni();
		void UpdateVariable(const TString& VarName, const TVarValue& VarValue);
	private:
		THMap<TString, TVarValue> Variables;
	};
}
