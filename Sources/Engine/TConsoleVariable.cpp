/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TConsoleVariable.h"
#include "TINIParser.h"

namespace tix
{
	TCVar::TCVar(const TString& VarName, int32& VarInt)
	{
		TConsoleVariables::AddCVar(VarName, &VarInt);
	}
	TCVar::TCVar(const TString& VarName, float& VarFloat)
	{
		TConsoleVariables::AddCVar(VarName, &VarFloat);
	}
	TCVar::TCVar(const TString& VarName, TString& VarString)
	{
		TConsoleVariables::AddCVar(VarName, &VarString);
	}

	///////////////////////////////////////////////////////////////////////
	TConsoleVariables* TConsoleVariables::s_cvar_instance = nullptr;
	THMap<TString, TVarMapping> TConsoleVariables::VarMap;

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
		VarMap.clear();

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
		TI_TODO("Read DefaultEngine.ini");
	}
}