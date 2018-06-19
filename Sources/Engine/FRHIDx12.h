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
	// Render hardware interface use DirectX 12
	class FRHIDx12 : public FRHI
	{
	public:
		virtual ~FRHIDx12();

		// RHI common methods
		virtual void BeginFrame() override;
		virtual void EndFrame() override;

		virtual FTexturePtr CreateTexture();
		virtual FTexturePtr CreateTexture(E_PIXEL_FORMAT Format, int32 Width, int32 Height);

		virtual FMeshBufferPtr CreateMeshBuffer(TMeshBufferPtr MeshBuffer) override;
		virtual bool UpdateHardwareBuffer(FTexturePtr Texture, TImagePtr InImage) override;

		// DirectX 12 specified methods
		void RecallDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE Handle);

	protected: 
		FRHIDx12();

		void Init();

	private:
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void CreateWindowsSizeDependentResources();
		void MoveToNextFrame();

		uint64 UpdateSubresources(
			_In_ ID3D12GraphicsCommandList* pCmdList,
			_In_ ID3D12Resource* pDestinationResource,
			_In_ ID3D12Resource* pIntermediate,
			_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
			_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources,
			uint64 RequiredSize,
			_In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT* pLayouts,
			_In_reads_(NumSubresources) const uint32* pNumRows,
			_In_reads_(NumSubresources) const uint64* pRowSizesInBytes,
			_In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA* pSrcData);
		uint64 UpdateSubresources(
			_In_ ID3D12GraphicsCommandList* pCmdList,
			_In_ ID3D12Resource* pDestinationResource,
			_In_ ID3D12Resource* pIntermediate,
			uint64 IntermediateOffset,
			_In_range_(0, D3D12_REQ_SUBRESOURCES) uint32 FirstSubresource,
			_In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) uint32 NumSubresources,
			_In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA* pSrcData);

		void Transition(
			_In_ ID3D12Resource* pResource,
			D3D12_RESOURCE_STATES stateBefore,
			D3D12_RESOURCE_STATES stateAfter,
			uint32 subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
			D3D12_RESOURCE_BARRIER_FLAGS flags = D3D12_RESOURCE_BARRIER_FLAG_NONE);

		void FlushResourceBarriers(
			_In_ ID3D12GraphicsCommandList* pCmdList);

		D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptorHandle();

	private:
		ComPtr<ID3D12Device>			D3dDevice;
		ComPtr<IDXGIFactory4>			DxgiFactory;
		ComPtr<IDXGISwapChain3>			SwapChain;
		ComPtr<ID3D12Resource>			BackBufferRTs[FRHI::FrameBufferNum];
		ComPtr<ID3D12Resource>			DepthStencil;
		ComPtr<ID3D12DescriptorHeap>	RtvHeap;
		ComPtr<ID3D12DescriptorHeap>	DsvHeap;
		ComPtr<ID3D12CommandQueue>		CommandQueue;
		ComPtr<ID3D12CommandAllocator>	CommandAllocators[FRHI::FrameBufferNum];
		ComPtr<ID3D12GraphicsCommandList>	CommandLists[FRHI::FrameBufferNum];

		// CPU/GPU Synchronization.
		ComPtr<ID3D12Fence>				Fence;
		uint64							FenceValues[FRHI::FrameBufferNum];
		HANDLE							FenceEvent;

		uint32							CurrentFrame;

		uint32							RtvDescriptorSize;

		static const uint32				MaxResourceBarrierBuffers = 16;
		D3D12_RESOURCE_BARRIER			ResourceBarrierBuffers[MaxResourceBarrierBuffers];
		uint32							NumBarriersToFlush;

		ComPtr<ID3D12DescriptorHeap>	SrvHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE		SrvHeapHandle;
		uint32							SrvDescriptorSize;
		TVector<D3D12_CPU_DESCRIPTOR_HANDLE> AvaibleSrvHeapHandles;

		friend class FRHI;
	};
}
#endif	// COMPILE_WITH_RHI_DX12