/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	TMaterialInstance::TMaterialInstance()
		: TResource(ERES_MATERIAL_INSTANCE)
		, ParamValueBuffer(0)
	{}

	TMaterialInstance::~TMaterialInstance()
	{
	}

	void TMaterialInstance::InitRenderThreadResource()
	{
		TI_TODO("Implement here.");
		TI_ASSERT(UniformBuffer == nullptr);
		UniformBuffer = FRHI::Get()->CreateUniformBuffer(EHT_UNIFORMBUFFER);
	}

	void TMaterialInstance::DestroyRenderThreadResource()
	{
		TI_TODO("Implement here.");
		UniformBuffer = nullptr;
	}

	static const int32 ParamTypeLength[MIPT_COUNT] =
	{
		0,	//MIPT_UNKNOWN,
		4,	//MIPT_INT,
		4,	//MIPT_FLOAT,
		16,	//MIPT_INT4,
		16,	//MIPT_FLOAT4,
		4,	//MIPT_TEXTURE (int),
	};

	int32 TMaterialInstance::GetParamTypeBytes(E_MI_PARAM_TYPE Type)
	{
		return ParamTypeLength[Type];
	}
}
