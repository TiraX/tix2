/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

#include <wrl.h>
#include <dxgi1_4.h>
#include <d3d12.h>
#include <pix.h>
#include "dx12/d3dx12.h"
#if defined(TIX_DEBUG)
#include <dxgidebug.h>
#endif

using namespace Microsoft::WRL;

namespace tix
{
	// Render hardware interface use DirectX 12
	class FRHIDx12 : public FRHI
	{
	public:
		virtual ~FRHIDx12();

		virtual void ClearBuffers() override;
	protected: 
		FRHIDx12();

		void Init();

	private:
		void GetHardwareAdapter(IDXGIAdapter1** ppAdapter);
		void CreateWindowsSizeDependentResources();

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

		// CPU/GPU Synchronization.
		ComPtr<ID3D12Fence>				Fence;
		uint64							FenceValues[FRHI::FrameBufferNum];
		HANDLE							FenceEvent;

		uint32							CurrentFrame;

		uint32							RtvDescriptorSize;

		friend class FRHI;
	};
}
#endif	// COMPILE_WITH_RHI_DX12