/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShaderBinding.h"

namespace tix
{
	FShaderBinding::FShaderBinding(uint32 InNumBindings)
		: FRenderResource(RRT_SHADER_BINDING)
		, NumBindings(InNumBindings)
		, MIArgumentsBindingIndex(-1)
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

	E_ARGUMENT_TYPE FShaderBinding::GetArgumentTypeByName(const TString& ArgName)
	{
		TString::size_type PrefixPos = ArgName.find('_');
		if (PrefixPos != TString::npos)
		{
			TString Prefix = ArgName.substr(0, PrefixPos);
			if (Prefix == "EB")
			{
				TString BufferName = ArgName.substr(PrefixPos + 1);
				// Engine Buffer
				if (BufferName == "View")
					return ARGUMENT_EB_VIEW;
				if (BufferName == "Primitive")
					return ARGUMENT_EB_PRIMITIVE;
				if (BufferName == "Lights")
					return ARGUMENT_EB_LIGHTS;
				if (BufferName == "IndirectTexture")
					return ARGUMENT_EB_VT_INDIRECT_AND_PHYSIC;
				if (BufferName == "PhysicPageAtlas")
					return ARGUMENT_EB_VT_INDIRECT_AND_PHYSIC;
				TI_ASSERT(0);
			}
		}

		return ARGUMENT_MI_ARGUMENTS;
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

	void FShaderBinding::AddShaderArgument(E_SHADER_STAGE ShaderStage, const FShaderArgument& InArgument)
	{
		if (ShaderStage == ESS_VERTEX_SHADER)
		{
			VertexArguments.push_back(InArgument);
		}
		else if (ShaderStage == ESS_PIXEL_SHADER)
		{
			PixelArguments.push_back(InArgument);
		}
		else
		{
			TI_ASSERT(0);
		}
	}

	void FShaderBinding::SortArguments()
	{
		TSort(VertexArguments.begin(), VertexArguments.end());
		TSort(PixelArguments.begin(), PixelArguments.end());
	}

	void FShaderBinding::PostInitArguments()
	{
		SortArguments();

		// Remember mi buffer binding index and mi texture binding index
		for (const auto& Arg : PixelArguments)
		{
			if (Arg.ArgumentType == ARGUMENT_MI_ARGUMENTS)
			{
				// Should only have 1 argument buffer binding for each shader, so we add a check here
				TI_ASSERT(MIArgumentsBindingIndex == -1);
				MIArgumentsBindingIndex = Arg.BindingIndex;
			}
		}
	}
}