/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDescriptorHeapDx12.h"
#include "FRHIDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	static const int32 MaxDescriptorCount[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		1024,	//EHT_CBV_SRV_UAV,
		512,	//EHT_SAMPLER,
		128,	//EHT_RTV,
		64,		//EHT_DSV,
	};

	static const D3D12_DESCRIPTOR_HEAP_FLAGS k_HEAP_CREATE_FLAGS[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,	//EHT_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_RTV,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_DSV,
	};

	static const LPCWSTR k_HEAP_NAMES[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
	{
		L"CBV_SRV_UAV_HEAP",	//EHT_CBV_SRV_UAV,
		L"SAMPLER_HEAP",	//EHT_SAMPLER,
		L"RTV_HEAP",	//EHT_RTV,
		L"DSV_HEAP",	//EHT_DSV,
	};

	FDescriptorHeapDx12::FDescriptorHeapDx12()
		: HeapType(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		, DescriptorIncSize(0)
	{
	}

	FDescriptorHeapDx12::~FDescriptorHeapDx12()
	{
	}

	static const E_RENDER_RESOURCE_HEAP_TYPE DxHeapMap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = 
	{
		EHT_UNIFORMBUFFER,	//D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV = 0,
		EHT_SAMPLER,		//D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER = (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV + 1) ,
		EHT_RENDERTARGET,	//D3D12_DESCRIPTOR_HEAP_TYPE_RTV = (D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1) ,
		EHT_DEPTHSTENCIL,	//D3D12_DESCRIPTOR_HEAP_TYPE_DSV = (D3D12_DESCRIPTOR_HEAP_TYPE_RTV + 1) ,
	};

	void FDescriptorHeapDx12::Create(FRHIDx12* RHIDx12, D3D12_DESCRIPTOR_HEAP_TYPE InHeapType)
	{
		HeapType = InHeapType;

		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = MaxDescriptorCount[InHeapType];
		HeapDesc.Type = InHeapType;
		HeapDesc.Flags = k_HEAP_CREATE_FLAGS[InHeapType];
		HeapDesc.NodeMask = 0;

		VALIDATE_HRESULT(RHIDx12->D3dDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap)));
		DescriptorHeap->SetName(k_HEAP_NAMES[InHeapType]);

		DescriptorIncSize = RHIDx12->D3dDevice->GetDescriptorHandleIncrementSize(InHeapType);

		if (InHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV)
		{
			TI_TODO("Remove these magic number.");
			RHIDx12->InitRHIRenderResourceHeap(EHT_UNIFORMBUFFER, 512, 0);
			RHIDx12->InitRHIRenderResourceHeap(EHT_TEXTURE, 448, 512);
			RHIDx12->InitRHIRenderResourceHeap(EHT_UNIFORMBUFFER_LIGHT, 64, 960);
		}
		else
		{
			RHIDx12->InitRHIRenderResourceHeap(DxHeapMap[InHeapType], MaxDescriptorCount[InHeapType], 0);
		}
	}

	D3D12_CPU_DESCRIPTOR_HANDLE FDescriptorHeapDx12::GetCpuDescriptorHandle(uint32 Index)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE Result = DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		Result.ptr += Index * DescriptorIncSize;
		return Result;
	}

	D3D12_GPU_DESCRIPTOR_HANDLE FDescriptorHeapDx12::GetGpuDescriptorHandle(uint32 Index)
	{
		D3D12_GPU_DESCRIPTOR_HANDLE Result = DescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		Result.ptr += Index * DescriptorIncSize;
		return Result;
	}
}
#endif	// COMPILE_WITH_RHI_DX12