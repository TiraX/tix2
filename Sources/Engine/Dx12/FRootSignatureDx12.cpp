/*
	TiX Engine v2.0 Copyright (C) 2018
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

	void FRootSignatureDx12::InitBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, uint32 InBindingRegisterIndex, uint32 InBindingSize, uint32 InBindingStage)
	{
#if DEBUG_SHADER_BINDING_TYPE
		InitBindingType(InBindingIndex, InBindingType);
#endif
		TI_ASSERT(InBindingStage < ESS_COUNT);
		D3D12_SHADER_VISIBILITY ShaderVisibility = Dx12ShaderVisibilityMap[InBindingStage];
		switch (InBindingType)
		{
		case BINDING_UNIFORMBUFFER:
			GetParameter(InBindingIndex).InitAsConstantBuffer(InBindingRegisterIndex, ShaderVisibility);
			break;
		case BINDING_UNIFORMBUFFER_TABLE:
			GetParameter(InBindingIndex).InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, InBindingRegisterIndex, InBindingSize, ShaderVisibility);
			break;
		case BINDING_TEXTURE_TABLE:
			GetParameter(InBindingIndex).InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, InBindingRegisterIndex, InBindingSize, ShaderVisibility);
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	void FRootSignatureDx12::InitStaticSampler(uint32 InBindingIndex, const FSamplerDesc& Desc, uint32 InBindingStage)
	{
		TI_ASSERT(InBindingStage < ESS_COUNT);
		D3D12_SHADER_VISIBILITY ShaderVisibility = Dx12ShaderVisibilityMap[InBindingStage];

		D3D12_SAMPLER_DESC Dx12Sampler = {};
		Dx12Sampler.Filter = GetDxTextureFilterFromTiX(Desc.Filter);
		Dx12Sampler.AddressU = GetDxTextureAddressModeFromTiX(Desc.AddressMode);
		Dx12Sampler.AddressV = GetDxTextureAddressModeFromTiX(Desc.AddressMode);
		Dx12Sampler.AddressW = GetDxTextureAddressModeFromTiX(Desc.AddressMode);
		Dx12Sampler.MipLODBias = 0;
		Dx12Sampler.MaxAnisotropy = 0;
		Dx12Sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		Dx12Sampler.MinLOD = 0.0f;
		Dx12Sampler.MaxLOD = D3D12_FLOAT32_MAX;

		D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		InitStaticSampler(InBindingIndex, Dx12Sampler, D3D12_SHADER_VISIBILITY_PIXEL);
	}

	void FRootSignatureDx12::Finalize(FRHI * RHI)
	{
		FRHIDx12 * RHIDx12 = static_cast<FRHIDx12*>(RHI);

		D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;

		Finalize(RHIDx12->GetD3dDevice().Get(), RootSignatureFlags);
	}

	void FRootSignatureDx12::InitStaticSampler(
		uint32 Register,
		const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
		D3D12_SHADER_VISIBILITY Visibility)
	{
		TI_ASSERT(NumInitializedStaticSamplers < SamplerArray.size());
		D3D12_STATIC_SAMPLER_DESC& StaticSamplerDesc = SamplerArray[NumInitializedStaticSamplers];
		++NumInitializedStaticSamplers;

		StaticSamplerDesc.Filter = NonStaticSamplerDesc.Filter;
		StaticSamplerDesc.AddressU = NonStaticSamplerDesc.AddressU;
		StaticSamplerDesc.AddressV = NonStaticSamplerDesc.AddressV;
		StaticSamplerDesc.AddressW = NonStaticSamplerDesc.AddressW;
		StaticSamplerDesc.MipLODBias = NonStaticSamplerDesc.MipLODBias;
		StaticSamplerDesc.MaxAnisotropy = NonStaticSamplerDesc.MaxAnisotropy;
		StaticSamplerDesc.ComparisonFunc = NonStaticSamplerDesc.ComparisonFunc;
		StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
		StaticSamplerDesc.MinLOD = NonStaticSamplerDesc.MinLOD;
		StaticSamplerDesc.MaxLOD = NonStaticSamplerDesc.MaxLOD;
		StaticSamplerDesc.ShaderRegister = Register;
		StaticSamplerDesc.RegisterSpace = 0;
		StaticSamplerDesc.ShaderVisibility = Visibility;

		if (StaticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
			StaticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
			StaticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
		{
			if (NonStaticSamplerDesc.BorderColor[3] == 1.0f)
			{
				if (NonStaticSamplerDesc.BorderColor[0] == 1.0f)
					StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
				else
					StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			}
			else
				StaticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		}
	}

	void FRootSignatureDx12::Finalize(ID3D12Device* D3dDevice, D3D12_ROOT_SIGNATURE_FLAGS Flags)
	{
		if (Finalized)
			return;

		TI_ASSERT(NumInitializedStaticSamplers == SamplerArray.size());

		D3D12_ROOT_SIGNATURE_DESC RootDesc;
		RootDesc.NumParameters = (uint32)ParamArray.size();
		RootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)(&ParamArray[0]);
		RootDesc.NumStaticSamplers = (uint32)SamplerArray.size();
		RootDesc.pStaticSamplers = SamplerArray.data();
		RootDesc.Flags = Flags;

		ComPtr<ID3DBlob> pOutBlob, pErrorBlob;
		
		VALIDATE_HRESULT(D3D12SerializeRootSignature(&RootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf()));

		VALIDATE_HRESULT(D3dDevice->CreateRootSignature(0, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(), IID_PPV_ARGS(&Signature)));

		Signature->SetName(L"RootSignature");
		
		Finalized = true;
	}
}
#endif	// COMPILE_WITH_RHI_DX12