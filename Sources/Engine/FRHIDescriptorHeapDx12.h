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
	enum E_HEAP_TYPE
	{
		EHT_INVALID = -1,
		EHT_CBV_SRV_UAV = 0,
		EHT_SAMPLER,
		EHT_RTV,
		EHT_DSV,

		EHT_COUNT,
	};

	class FDescriptorHeapDx12
	{
	public:
		FDescriptorHeapDx12();

		void Create(ID3D12Device* D3dDevice, E_HEAP_TYPE HeapType);
		void Destroy();

		uint32 AllocateDescriptorSlot();
		D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor();
		void RecallDescriptor(uint32 InHeapIndex);
		void RecallDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE InHeapIndex);

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
		TVector<uint32> AvaibleDescriptorHeapSlots;
		uint32 DescriptorAllocated;
	};
}
#endif	// COMPILE_WITH_RHI_DX12