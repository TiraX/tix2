/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FUniformBuffer.h"

namespace tix
{
	BEGIN_UNIFORM_BUFFER_STRUCT(FViewUniformBuffer)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(matrix4, ViewProjection)
	END_UNIFORM_BUFFER_STRUCT(FViewUniformBuffer)
}
