/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

#include "dx12/d3dx12.h"

using namespace Microsoft::WRL;

namespace tix
{
#define MAX_CBV_SRV_UAV_DESCRIPTORS 1024
#define LIGHTS_IN_CBV_SRV_UAV_HEAP 64
	enum E_HEAP_TYPE
	{
		EHT_INVALID = -1,
		EHT_CBV_SRV_UAV = 0,
		EHT_SAMPLER,
		EHT_RTV,
		EHT_DSV,

		EHT_COUNT,
	};

	class FDescriptorHeapDx12;
	class FDescriptorAllocator
	{
	public:
		FDescriptorAllocator(FDescriptorHeapDx12* InHeap, uint32 InOffset, uint32 InSize)
			: Heap(InHeap)
			, DescriptorAllocated(0)
			, OffsetInHeap(InOffset)
			, Size(InSize)
		{}
		~FDescriptorAllocator() {}

		uint32 AllocateDescriptorSlot();
		D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor();
		void RecallDescriptor(uint32 InHeapIndex);
		void RecallDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE InHeapIndex);

	private:
		FDescriptorHeapDx12 * Heap;
		TVector<uint32> AvaibleDescriptorHeapSlots;
		uint32 DescriptorAllocated;
		uint32 OffsetInHeap;
		uint32 Size;
	};

	class FDescriptorHeapDx12
	{
	public:
		FDescriptorHeapDx12();
		~FDescriptorHeapDx12();

		void Create(ID3D12Device* D3dDevice, E_HEAP_TYPE HeapType);

		void ClearAllAllocators();
		void CreateDefaultAllocator();
		void CreateAllocator(uint32 Offset, uint32 Size);

		FDescriptorAllocator* GetAllocator(int32 Index)
		{
			return Allocators[Index];
		}

		FDescriptorAllocator* GetDefaultAllocator()
		{
			return GetAllocator(0);
		}

		D3D12_CPU_DESCRIPTOR_HANDLE GetCpuDescriptorHandle(uint32 Index);
		D3D12_GPU_DESCRIPTOR_HANDLE GetGpuDescriptorHandle(uint32 Index);
		
		uint32 GetIncSize() const
		{
			return DescriptorIncSize;
		}

		ID3D12DescriptorHeap* GetHeap()
		{
			return DescriptorHeap.Get();
		}
	private:
		E_HEAP_TYPE HeapType;
		ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
		uint32 DescriptorIncSize;
		TVector<FDescriptorAllocator*> Allocators;
	};
}
#endif	// COMPILE_WITH_RHI_DX12