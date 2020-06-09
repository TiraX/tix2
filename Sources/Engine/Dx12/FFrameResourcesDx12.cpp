/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FFrameResourcesDx12.h"

#if COMPILE_WITH_RHI_DX12
namespace tix
{
	FFrameResourcesDx12::FFrameResourcesDx12()
	{
		//D3d12Resources.reserve(FFrameResources::DefaultReserveCount);
	}

	FFrameResourcesDx12::~FFrameResourcesDx12()
	{
	}

	void FFrameResourcesDx12::RemoveAllReferences()
	{
		FFrameResources::RemoveAllReferences();

		for (auto& R : D3d12Resources)
		{
			R = nullptr;
		}
		D3d12Resources.clear();
	}

	void FFrameResourcesDx12::HoldDxReference(ComPtr<ID3D12Resource> Res)
	{
		D3d12Resources.push_back(Res);
	}
}
#endif	//COMPILE_WITH_RHI_DX12