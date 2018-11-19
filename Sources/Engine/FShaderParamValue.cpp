/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FShaderParamValue.h"
#include "FShaderBinding.h"

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
		ShaderBinding->ValidateBinding(InBindingIndex, BINDING_UNIFORMBUFFER, false);
#endif
	}

	void FShaderParamValue::SetParamValue(uint32 InBindingIndex, FTexturePtr Texture)
	{
#if DEBUG_SHADER_BINDING_TYPE
		ShaderBinding->ValidateBinding(InBindingIndex, BINDING_TEXTURE, false);
#endif
	}

	void FShaderParamValue::SetParamValue(uint32 InBindingIndex, FRenderResourceTablePtr RenderResourceTable)
	{
#if DEBUG_SHADER_BINDING_TYPE
		
#endif
		RenderResourceTable->GetHeapType();
		TI_ASSERT(0);
	}
}