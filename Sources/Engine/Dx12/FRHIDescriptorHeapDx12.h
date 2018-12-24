/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

#include "d3dx12.h"

using namespace Microsoft::WRL;

namespace tix
{
	class FRHIDx12;
	class FDescriptorHeapDx12
	{
	public:
		FDescriptorHeapDx12();
		~FDescriptorHeapDx12();

		// Create dx12 descriptor heap, and init TiX RHI Render Resource Heap
		void Create(FRHIDx12* RHIDx12, D3D12_DESCRIPTOR_HEAP_TYPE HeapType);

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
		D3D12_DESCRIPTOR_HEAP_TYPE HeapType;
		ComPtr<ID3D12DescriptorHeap> DescriptorHeap;
		uint32 DescriptorIncSize;
	};
}
#endif	// COMPILE_WITH_RHI_DX12