/*
	TiX Engine v2.0 Copyright (C) 2018
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
	static const DXGI_FORMAT SematicFormat[ESSI_TOTAL] = 
	{
		DXGI_FORMAT_R32G32B32_FLOAT,	// ESSI_POSITION,
		DXGI_FORMAT_R32G32B32_FLOAT,	// ESSI_NORMAL,		// TI_TODO("May use packed normal in future");
		DXGI_FORMAT_B8G8R8A8_UNORM,		// ESSI_COLOR,
		DXGI_FORMAT_R32G32_FLOAT,		// ESSI_TEXCOORD0,	// TI_TODO("May use half float texcoord in future");
		DXGI_FORMAT_R32G32_FLOAT,		// ESSI_TEXCOORD1,	// TI_TODO("May use half float texcoord in future");
		DXGI_FORMAT_R32G32B32_FLOAT,	// ESSI_TANGENT,	// TI_TODO("May use packed tangent in future");
		DXGI_FORMAT_R8G8B8A8_UINT,		// ESSI_BLENDINDEX,
		DXGI_FORMAT_R32G32B32A32_FLOAT,	// ESSI_BLENDWEIGHT,// TI_TODO("May use half float blend weight in future");
	};
	FMeshBufferDx12::FMeshBufferDx12()
	{
	}

	FMeshBufferDx12::~FMeshBufferDx12()
	{
		Destroy();
	}

	void FMeshBufferDx12::Destroy()
	{
		TI_TODO("Destroy() function seems can be removed.");
		TI_ASSERT(IsRenderThread());
		VertexBuffer = nullptr;
		IndexBuffer = nullptr;
	}
}

#endif	// COMPILE_WITH_RHI_DX12