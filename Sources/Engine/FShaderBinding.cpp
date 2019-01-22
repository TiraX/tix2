/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShaderBinding.h"

namespace tix
{
	FShaderBinding::FShaderBinding(uint32 InNumBindings)
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
	void FShaderBinding::InitBindingType(uint32 InBindingIndex, E_BINDING_TYPE InBindingType)
	{
		TI_ASSERT(BindingTypes[InBindingIndex] == BINDING_TYPE_INVALID);
		BindingTypes[InBindingIndex] = InBindingType;
	}

	void FShaderBinding::ValidateBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType)
	{
		TI_ASSERT(BindingTypes[InBindingIndex] != BINDING_TYPE_INVALID);
		TI_ASSERT(BindingTypes[InBindingIndex] == InBindingType);
	}
#endif
}