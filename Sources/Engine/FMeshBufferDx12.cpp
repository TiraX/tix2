/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
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
	static const int32 SematicSize[ESSI_TOTAL] =
	{
		12,	// ESSI_POSITION,
		12,	// ESSI_NORMAL,		// TI_TODO("May use packed normal in future");
		4,	// ESSI_COLOR,
		8,	// ESSI_TEXCOORD0,	// TI_TODO("May use half float texcoord in future");
		8,	// ESSI_TEXCOORD1,	// TI_TODO("May use half float texcoord in future");
		12,	// ESSI_TANGENT,	// TI_TODO("May use packed tangent in future");
		4,	// ESSI_BLENDINDEX,
		16,	// ESSI_BLENDWEIGHT,// TI_TODO("May use half float blend weight in future");
	};
	FMeshBufferDx12::FMeshBufferDx12()
		: FMeshBuffer(EMBT_Dx12)
	{
	}

	FMeshBufferDx12::~FMeshBufferDx12()
	{
	}

	void FMeshBufferDx12::CreateHardwareBuffer() 
	{
		// Create Layout
		TVector<D3D12_INPUT_ELEMENT_DESC> Layout;
		GetLayout(Layout);


	}

	void FMeshBufferDx12::GetLayout(TVector<D3D12_INPUT_ELEMENT_DESC>& Layout)
	{
		int32 Segments = 0;
		for (uint32 seg = 1; seg <= EVSSEG_TOTAL; seg <<= 1)
		{
			if (VsFormat & seg)
			{
				++Segments;
			}
		}
		Layout.resize(Segments);
		for (uint32 seg = 1, i = 0; seg <= EVSSEG_TOTAL; seg <<= 1, ++i)
		{
			if (VsFormat & seg)
			{
				int32 SemanticOffset = 0;
				D3D12_INPUT_ELEMENT_DESC& Desc = Layout[i];
				Desc.SemanticName = SemanticName[i];
				Desc.SemanticIndex = i;
				Desc.Format = SematicFormat[i];
				Desc.InputSlot = 0;
				Desc.AlignedByteOffset = SemanticOffset;
				Desc.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
				Desc.InstanceDataStepRate = 0;

				SemanticOffset += SematicSize[i];
			}
		}
	}
}

#endif	// COMPILE_WITH_RHI_DX12