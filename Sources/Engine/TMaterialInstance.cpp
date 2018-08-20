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
		TI_ASSERT(0);
	}

	void TMaterialInstance::DestroyRenderThreadResource()
	{
		TI_TODO("Implement here.");
		TI_ASSERT(0);
	}

	static const int32 ParamTypeLength[MIPT_COUNT] =
	{
		0,	//MIPT_UNKNOWN,
		4,	//MIPT_INT,
		4,	//MIPT_FLOAT,
		16,	//MIPT_INT4,
		16,	//MIPT_FLOAT4,
		0,	//MIPT_TEXTURE,
	};
	int32 TMaterialInstance::GetValueBufferLength() const
	{
		if (ParamValueBuffer.GetLength() > 0)
			return ParamValueBuffer.GetLength();

		// Calculate length from param types
		int32 Length = 0;
		for (auto t : ParamTypes)
		{
			TI_ASSERT(t > MIPT_UNKNOWN && t < MIPT_COUNT);
			Length += ParamTypeLength[t];
		}
		return Length;
	}
}
