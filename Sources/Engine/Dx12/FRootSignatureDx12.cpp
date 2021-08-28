/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#if COMPILE_WITH_RHI_DX12
#include "FRootSignatureDx12.h"
#include "FRHIDx12.h"
#include "FRHIDx12Conversion.h"

namespace tix
{
	static const D3D12_SHADER_VISIBILITY Dx12ShaderVisibilityMap[ESS_COUNT] =
	{
		D3D12_SHADER_VISIBILITY_VERTEX,	//ESS_VERTEX_SHADER,
		D3D12_SHADER_VISIBILITY_PIXEL,	//ESS_PIXEL_SHADER,
		D3D12_SHADER_VISIBILITY_DOMAIN,	//ESS_DOMAIN_SHADER,
		D3D12_SHADER_VISIBILITY_HULL,	//ESS_HULL_SHADER,
		D3D12_SHADER_VISIBILITY_GEOMETRY,	//ESS_GEOMETRY_SHADER,
	};

	void FRootSignatureDx12::Finalize(ID3D12Device* D3dDevice, const D3D12_ROOT_SIGNATURE_DESC& RSDesc)
	{
		if (Finalized)
			return;

		ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

		VALIDATE_HRESULT(D3D12SerializeRootSignature(&RSDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf()));

		VALIDATE_HRESULT(D3dDevice->CreateRootSignature(0, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(), IID_PPV_ARGS(&Signature)));

		Signature->SetName(L"RootSignature");

		Finalized = true;
	}
}
#endif	// COMPILE_WITH_RHI_DX12