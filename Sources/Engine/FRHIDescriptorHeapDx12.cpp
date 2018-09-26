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
		MAX_CBV_SRV_UAV_DESCRIPTORS,	//EHT_CBV_SRV_UAV,
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
	{
	}

	FDescriptorHeapDx12::~FDescriptorHeapDx12()
	{
		ClearAllAllocators();
	}

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

		CreateDefaultAllocator();
	}

	void FDescriptorHeapDx12::ClearAllAllocators()
	{
		for (auto& i : Allocators)
		{
			ti_delete i;
			i = nullptr;
		}
		Allocators.clear();
	}

	void FDescriptorHeapDx12::CreateDefaultAllocator()
	{
		FDescriptorAllocator* Allocator = ti_new FDescriptorAllocator(this, 0, MaxDescriptorCount[HeapType]);
		Allocators.push_back(Allocator);
	}

	void FDescriptorHeapDx12::CreateAllocator(uint32 Offset, uint32 Size)
	{
		TI_ASSERT(Offset + Size <= (uint32)MaxDescriptorCount[HeapType]);
		FDescriptorAllocator* Allocator = ti_new FDescriptorAllocator(this, Offset, Size);
		Allocators.push_back(Allocator);
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

	/////////////////////////////////////////////////////////////////////////////////


	D3D12_CPU_DESCRIPTOR_HANDLE FDescriptorAllocator::AllocateDescriptor()
	{
		uint32 SlotIndex = AllocateDescriptorSlot();
		return Heap->GetCpuDescriptorHandle(SlotIndex);
	}

	void FDescriptorAllocator::RecallDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE Descriptor)
	{
		D3D12_CPU_DESCRIPTOR_HANDLE Start = Heap->GetCpuDescriptorHandle(0);
		int32 IndexOffset = (int32)(Descriptor.ptr - Start.ptr);
		TI_ASSERT(IndexOffset >= 0 && (IndexOffset % Heap->GetIncSize()) == 0);
		uint32 Index = IndexOffset / Heap->GetIncSize();
		RecallDescriptor(Index);
	}

	uint32 FDescriptorAllocator::AllocateDescriptorSlot()
	{
		if (AvaibleDescriptorHeapSlots.size() > 0)
		{
			uint32 SlotIndex = AvaibleDescriptorHeapSlots.back();
			AvaibleDescriptorHeapSlots.pop_back();
			return SlotIndex;
		}
		uint32 Result = DescriptorAllocated + OffsetInHeap;
		++DescriptorAllocated;
		TI_ASSERT(DescriptorAllocated < Size);
		return Result;
	}

	void FDescriptorAllocator::RecallDescriptor(uint32 HeapIndex)
	{
		AvaibleDescriptorHeapSlots.push_back(HeapIndex);
	}
}
#endif	// COMPILE_WITH_RHI_DX12