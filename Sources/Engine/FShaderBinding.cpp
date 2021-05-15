/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShaderBinding.h"

namespace tix
{
	FShaderBinding::FShaderBinding(uint32 InNumBindings)
		: FRenderResource(RRT_SHADER_BINDING)
		, NumBindings(InNumBindings)
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
				if (BufferName == "Bones")
					return ARGUMENT_EB_BONES;
				if (BufferName == "Lights")
					return ARGUMENT_EB_LIGHTS;
				if (BufferName == "EnvCube")
					return ARGUMENT_EB_ENV_CUBE;
#if (COMPILE_WITH_RHI_METAL)
                if (BufferName == "VTIndirectTexture")
                    return ARGUMENT_EB_VT_INDIRECT;
                if (BufferName == "VTPhysicTexture")
                    return ARGUMENT_EB_VT_PHYSIC;
#else
                if (BufferName.substr(0, 6) == "VTArgs")
                    return ARGUMENT_EB_VT_INDIRECT_AND_PHYSIC;
#endif
			}
            else if (Prefix == "MI")
            {
                return ARGUMENT_MI_ARGUMENTS;
            }
            TI_ASSERT(0);
		}

		return ARGUMENT_UNKNOWN;
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
            // Vertex / Compute shader arguments share the same container
			VertexComputeArguments.push_back(InArgument);
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
		TSort(VertexComputeArguments.begin(), VertexComputeArguments.end());
		TSort(PixelArguments.begin(), PixelArguments.end());
	}

	void FShaderBinding::PostInitArguments()
	{
		SortArguments();
	}
}
