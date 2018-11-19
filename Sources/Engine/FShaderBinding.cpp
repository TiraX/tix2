/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShaderBinding.h"

namespace tix
{
	FShaderBinding::FShaderBinding(uint32 InNumBindings, uint32 NumStaticSamplers)
		: NumBindings(InNumBindings)
	{
#if DEBUG_SHADER_BINDING_TYPE
		BindingTypes.resize(InNumBindings);
		for (uint32 i = 0 ; i < InNumBindings; ++ i)
		{
			BindingTypes[i] = BINDING_TYPE_INVALID;
		}
#endif
	}

	FShaderBinding::~FShaderBinding()
	{
	}

#if DEBUG_SHADER_BINDING_TYPE
	static const uint32 BindingTableMark = 1 << 7;
	void FShaderBinding::InitBindingType(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, bool IsTable)
	{
		TI_ASSERT(BindingTypes[InBindingType] == BINDING_TYPE_INVALID);
		uint32 BindingType = InBindingType;
		if (IsTable)
		{
			BindingType += BindingTableMark;
		}
		BindingTypes[InBindingIndex] = BindingType;
	}

	void FShaderBinding::ValidateBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, bool IsTable)
	{
		TI_ASSERT(BindingTypes[InBindingType] != BINDING_TYPE_INVALID);
		uint32 BindingType = InBindingType;
		if (IsTable)
		{
			BindingType += BindingTableMark;
		}
		TI_ASSERT(BindingTypes[InBindingType] == BindingType);
	}
#endif
}