/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShaderParamValue.h"

namespace tix
{
	FShaderParamValue::FShaderParamValue(FShaderBindingPtr InBinding)
		: ShaderBinding(InBinding)
	{
		ShaderValues.resize(InBinding->GetNumBinding());
	}

	FShaderParamValue::~FShaderParamValue()
	{
	}

	void FShaderParamValue::SetParamValue(uint32 InBindingIndex, FUniformBufferPtr UniformBuffer)
	{
#if DEBUG_SHADER_BINDING_TYPE
		ShaderBinding->ValidateBinding(InBindingIndex, BINDING_UNIFORMBUFFER);
#endif
		ShaderValues[InBindingIndex] = FShaderValue(UniformBuffer, BINDING_UNIFORMBUFFER);
	}

	void FShaderParamValue::SetParamValue(uint32 InBindingIndex, FTexturePtr Texture)
	{
		TI_ASSERT(0);
	}

	void FShaderParamValue::SetParamValue(uint32 InBindingIndex, FRenderResourceTablePtr RenderResourceTable)
	{
		E_BINDING_TYPE BindingType = BINDING_TYPE_INVALID;
		E_RENDER_RESOURCE_HEAP_TYPE HeapType = RenderResourceTable->GetHeapType();
		BindingType = (HeapType == EHT_UNIFORMBUFFER) ? BINDING_UNIFORMBUFFER_TABLE : BINDING_TEXTURE_TABLE;
#if DEBUG_SHADER_BINDING_TYPE
		ShaderBinding->ValidateBinding(InBindingIndex, BindingType);
#endif
		ShaderValues[InBindingIndex] = FShaderValue(RenderResourceTable, BindingType);
	}

	void FShaderParamValue::ApplyParamValues(FRHI * RHI)
	{
		const int32 Values = (int32)ShaderValues.size();
		for (int32 i = 0 ; i < Values ; ++ i)
		{
			FShaderValue& Value = ShaderValues[i];
			switch (Value.BindingType)
			{
			case BINDING_UNIFORMBUFFER:
			{
				FUniformBufferPtr UniformBuffer = ResourceCast<FUniformBuffer>(Value.RenderResource);
				RHI->SetUniformBuffer(i, UniformBuffer);
			}
			break;
			case BINDING_UNIFORMBUFFER_TABLE:
			case BINDING_TEXTURE_TABLE:
			{
				FRenderResourceTablePtr ResourceTable = ResourceCast<FRenderResourceTable>(Value.RenderResource);
				RHI->SetRenderResourceTable(i, ResourceTable);
			}
			break;
			default:
				TI_ASSERT(0);
				break;
			}
		}
	}
}