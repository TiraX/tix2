/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#if COMPILE_WITH_RHI_DX12
#include "FGPUResourceDx12.h"
#include "FRHIDx12.h"

namespace tix
{
	FGPUResourceDx12::FGPUResourceDx12()
		: UsageState(D3D12_RESOURCE_STATE_COMMON)
	{}

	FGPUResourceDx12::FGPUResourceDx12(D3D12_RESOURCE_STATES InitState)
		: UsageState(InitState)
	{}

	FGPUResourceDx12::~FGPUResourceDx12()
	{
		Resource = nullptr;
	}

	void FGPUResourceDx12::CreateResource(
		ID3D12Device* Device,
		const D3D12_HEAP_PROPERTIES *pHeapProperties,
		D3D12_HEAP_FLAGS HeapFlags,
		const D3D12_RESOURCE_DESC *pDesc,
		D3D12_RESOURCE_STATES InitialResourceState,
		const D3D12_CLEAR_VALUE *pOptimizedClearValue)
	{
		Resource = nullptr;
		UsageState = InitialResourceState;
		VALIDATE_HRESULT(Device->CreateCommittedResource(
			pHeapProperties,
			HeapFlags,
			pDesc,
			InitialResourceState,
			pOptimizedClearValue,
			IID_PPV_ARGS(&Resource)));
	}
}
#endif	// COMPILE_WITH_RHI_DX12