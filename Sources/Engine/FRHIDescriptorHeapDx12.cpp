/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDescriptorHeapDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	static const int32 MaxDescriptorCount[EHT_COUNT] = 
	{
		1024,	//EHT_CBV_SRV_UAV,
		512,	//EHT_SAMPLER,
		128,	//EHT_RTV,
		64,		//EHT_DSV,
	};

	static const D3D12_DESCRIPTOR_HEAP_TYPE k_DESCRIPTOR_HEAP_TYPE_MAP[EHT_COUNT] =
	{
		D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,	//EHT_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,	//EHT_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_TYPE_RTV,		//EHT_RTV,
		D3D12_DESCRIPTOR_HEAP_TYPE_DSV,		//EHT_DSV,
	};

	static const D3D12_DESCRIPTOR_HEAP_FLAGS k_HEAP_CREATE_FLAGS[EHT_COUNT] =
	{
		D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,	//EHT_CBV_SRV_UAV,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_SAMPLER,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_RTV,
		D3D12_DESCRIPTOR_HEAP_FLAG_NONE,	//EHT_DSV,
	};

	static const LPCWSTR k_HEAP_NAMES[EHT_COUNT] = 
	{
		L"CBV_SRV_UAV_HEAP",	//EHT_CBV_SRV_UAV,
		L"SAMPLER_HEAP",	//EHT_SAMPLER,
		L"RTV_HEAP",	//EHT_RTV,
		L"DSV_HEAP",	//EHT_DSV,
	};

	FDescriptorHeapDx12::FDescriptorHeapDx12()
		: HeapType(EHT_INVALID)
		, DescriptorIncSize(0)
		, DescriptorAllocated(0)
	{}

	void FDescriptorHeapDx12::Create(ID3D12Device* D3dDevice, E_HEAP_TYPE InHeapType)
	{
		HeapType = InHeapType;

		D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
		HeapDesc.NumDescriptors = MaxDescriptorCount[InHeapType];
		HeapDesc.Type = k_DESCRIPTOR_HEAP_TYPE_MAP[InHeapType];
		HeapDesc.Flags = k_HEAP_CREATE_FLAGS[InHeapType];
		HeapDesc.NodeMask = 0;

		VALIDATE_HRESULT(D3dDevice->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&DescriptorHeap)));
		DescriptorHeap->SetName(k_HEAP_NAMES[InHeapType]);

		DescriptorIncSize = D3dDevice->GetDescriptorHandleIncrementSize(k_DESCRIPTOR_HEAP_TYPE_MAP[InHeapType]);
	}

	void FDescriptorHeapDx12::Destroy()
	{

	}

	uint32 FDescriptorHeapDx12::AllocateDescriptorSlot()
	{
		if (AvaibleDescriptorHeapSlots.size() > 0)
		{
			uint32 SlotIndex = AvaibleDescriptorHeapSlots.back();
			AvaibleDescriptorHeapSlots.pop_back();
			return SlotIndex;
		}
		uint32 Result = DescriptorAllocated;
		++DescriptorAllocated;
		TI_ASSERT(DescriptorAllocated < (uint32)MaxDescriptorCount[HeapType]);
		return Result;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE FDescriptorHeapDx12::AllocateDescriptor()
	{
		uint32 SlotIndex = AllocateDescriptorSlot();
		return GetCpuDescriptorHandle(SlotIndex);
	}

	void FDescriptorHeapDx12::RecallDescriptor(uint32 HeapIndex)
	{
		AvaibleDescriptorHeapSlots.push_back(HeapIndex);
	}

	void FDescriptorHeapDx12::RecallDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE Descriptor)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE Start = DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		int32 Offset = Descriptor.ptr - Start.ptr;
		TI_ASSERT(Offset >= 0 && (Offset % DescriptorIncSize) == 0);
		uint32 Index = Offset / DescriptorIncSize;
		RecallDescriptor(Index);
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