/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#if COMPILE_WITH_RHI_METAL
#include "FShaderBindingMetal.h"
#include "FRHIMetal.h"

namespace tix
{
    void FShaderBindingMetal::InitBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, uint32 InBindingRegisterIndex, uint32 InBindingSize, uint32 InBindingStage)
	{
	}

	void FShaderBindingMetal::InitStaticSampler(uint32 InBindingIndex, const FSamplerDesc& Desc, uint32 InBindingStage)
	{
	}

	void FShaderBindingMetal::Finalize(FRHI * RHI)
	{
	}
}
#endif	// COMPILE_WITH_RHI_METAL
