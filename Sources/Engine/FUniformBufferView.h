/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FUniformBuffer.h"

namespace tix
{
	BEGIN_UNIFORM_BUFFER_STRUCT(FViewUniformBuffer, EHT_UNIFORMBUFFER)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, ViewProjection)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FVector4, ViewDir)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FVector4, ViewPos)
	END_UNIFORM_BUFFER_STRUCT(FViewUniformBuffer)
}
