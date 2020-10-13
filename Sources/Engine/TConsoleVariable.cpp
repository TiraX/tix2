/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TConsoleVariable.h"

namespace tix
{
	TCVar::TCVar(const TString& VarName, int32& VarInt, uint32 InFlag)
		: CVarName(VarName)
		, ValueInt(&VarInt)
		, ValueFloat(nullptr)
		, ValueString(nullptr)
		, ValueType(VAR_INT)
		, Flag(InFlag)
	{
		TConsoleVariables::AddCVar(VarName, this);
	}
	TCVar::TCVar(const TString& VarName, float& VarFloat, uint32 InFlag)
		: CVarName(VarName)
		, ValueInt(nullptr)
		, ValueFloat(&VarFloat)
		, ValueString(nullptr)
		, ValueType(VAR_FLOAT)
		, Flag(InFlag)
	{
		TConsoleVariables::AddCVar(VarName, this);
	}
	TCVar::TCVar(const TString& VarName, TString& VarString, uint32 InFlag)
		: CVarName(VarName)
		, ValueInt(nullptr)
		, ValueFloat(nullptr)
		, ValueString(&VarString)
		, ValueType(VAR_STRING)
		, Flag(InFlag)
	{
		TConsoleVariables::AddCVar(VarName, this);
	}

	inline const int8* GetTypeString(int32 Type)
	{
		switch (Type)
		{
		case VAR_INT:
			return "VAR_INT";
			break;
		case VAR_FLOAT:
			return "VAR_FLOAT";
			break;
		case VAR_STRING:
			return "VAR_STRING";
			break;
		}
		return "VAR_UNKNOWN";
	}

	void TCVar::ValueUpdated(const TVarValue& Value)
	{
		TI_TODO("Add cvar set from xxx.");
		if (ValueType == Value.VType)
		{
			switch (ValueType)
			{
			case VAR_INT:
				TI_ASSERT(ValueInt != nullptr);
				*ValueInt = Value.VInt;
				break;
			case VAR_FLOAT:
				TI_ASSERT(ValueFloat != nullptr);
				*ValueFloat = Value.VFloat;
				break;
			case VAR_STRING:
				TI_ASSERT(ValueString != nullptr);
				*ValueString = Value.VString;
				break;
			default:
				TI_ASSERT(0);
				break;
			}
		}
		else
		{
			_LOG(Warning, "CVar %s, set value %s, require %s.\n", CVarName.c_str(), GetTypeString(Value.VType), GetTypeString(ValueType));
			switch (ValueType)
			{
			case VAR_INT:
				TI_ASSERT(ValueInt != nullptr);
				TI_ASSERT(Value.VType == VAR_INT || Value.VType == VAR_FLOAT);
				*ValueInt = Value.VInt;
				break;
			case VAR_FLOAT:
				TI_ASSERT(ValueFloat != nullptr);
				TI_ASSERT(Value.VType == VAR_INT || Value.VType == VAR_FLOAT);
				*ValueFloat = Value.VFloat;
				break;
			default:
				TI_ASSERT(0);
				break;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////
	TConsoleVariables* TConsoleVariables::s_cvar_instance = nullptr;

	void TConsoleVariables::Init()
	{
		if (s_cvar_instance == nullptr)
		{
			s_cvar_instance = ti_new TConsoleVariables;
			s_cvar_instance->InitFromIni();
		}
	}

	void TConsoleVariables::Destroy()
	{
		TI_ASSERT(s_cvar_instance);
		SAFE_DELETE(s_cvar_instance);
	}

	TConsoleVariables* TConsoleVariables::Get()
	{
		return s_cvar_instance;
	}

	TConsoleVariables::TConsoleVariables()
	{
	}

	TConsoleVariables::~TConsoleVariables()
	{
	}

	void TConsoleVariables::InitFromIni()
	{
		TINIParser Parser("Config/DefaultEngine.ini");

		TINIData Data;
		Parser.Parse(Data);

		for (const auto& V : Data)
		{
			size_t Sep = V.first.rfind('.');
			TString Section = V.first.substr(0, Sep);
			TString Name = V.first.substr(Sep + 1);
			UpdateVariable(Name, V.second);
		}
	}

	void TConsoleVariables::UpdateVariable(const TString& VarName, const TVarValue& VarValue)
	{
		Variables[VarName] = VarValue;

		THMap<TString, TCVar*>& VarMap = GetVarMap();
		auto it = VarMap.find(VarName);
		if (it != VarMap.end())
		{
			it->second->ValueUpdated(VarValue);
		}
	}
}