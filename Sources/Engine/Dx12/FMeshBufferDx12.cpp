/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FMeshBufferDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	static const char* SemanticName[ESSI_TOTAL] =
	{
		"POSITION",	// ESSI_POSITION,
		"NORMAL",	// ESSI_NORMAL,
		"COLOR",	// ESSI_COLOR,
		"TEXCOORD0",	// ESSI_TEXCOORD0,
		"TEXCOORD1",	// ESSI_TEXCOORD1,
		"TANGENT",	// ESSI_TANGENT,
		"BLENDINDEX",	// ESSI_BLENDINDEX,
		"BLENDWEIGHT",	// ESSI_BLENDWEIGHT,
	};
	FMeshBufferDx12::FMeshBufferDx12()
	{
	}

	FMeshBufferDx12::~FMeshBufferDx12()
	{
		TI_ASSERT(IsRenderThread());
		VertexBuffer = nullptr;
		IndexBuffer = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12