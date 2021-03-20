/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"

using namespace Microsoft::WRL;

namespace tix
{
	class FRHIDXR
	{
	private:
		FRHIDXR();
		~FRHIDXR() {};

		bool Init(ComPtr<ID3D12Device> D3DDevice);

	private:
		// DirectX Raytracing (DXR) attributes
		ComPtr<ID3D12Device5> DXRDevice;
		ComPtr<ID3D12GraphicsCommandList4> DXRCommandList;
		friend class FRHIDx12;
	};
}
#endif	// COMPILE_WITH_RHI_DX12