/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

#include "d3dx12.h"

using namespace Microsoft::WRL;

namespace tix
{
	class FRHIDx12;

	class FRootParameterDx12
	{
	public:

		FRootParameterDx12()
		{
			RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
		}

		~FRootParameterDx12()
		{
			Clear();
		}

		void Clear()
		{
			if (RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
				ti_delete[] RootParam.DescriptorTable.pDescriptorRanges;

			RootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
		}

		void InitAsConstants(uint32 Register, uint32 NumDwords, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			RootParam.ShaderVisibility = Visibility;
			RootParam.Constants.Num32BitValues = NumDwords;
			RootParam.Constants.ShaderRegister = Register;
			RootParam.Constants.RegisterSpace = 0;
		}

		void InitAsConstantBuffer(uint32 Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsBuffer(D3D12_ROOT_PARAMETER_TYPE_CBV, Register, Visibility);
		}

		void InitAsBufferSRV(uint32 Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsBuffer(D3D12_ROOT_PARAMETER_TYPE_SRV, Register, Visibility);
		}

		void InitAsBufferUAV(uint32 Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			InitAsBuffer(D3D12_ROOT_PARAMETER_TYPE_UAV, Register, Visibility);
		}

		void InitAsBuffer(D3D12_ROOT_PARAMETER_TYPE ParamType, uint32 Register, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			RootParam.ParameterType = ParamType;
			RootParam.ShaderVisibility = Visibility;
			RootParam.Descriptor.ShaderRegister = Register;
			RootParam.Descriptor.RegisterSpace = 0;
		}

		void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE Type, uint32 Register, uint32 Count, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			D3D12_DESCRIPTOR_RANGE* Ranges = ti_new D3D12_DESCRIPTOR_RANGE;
			SetTableRange(Ranges, Type, Register, Count);
			InitAsDescriptorTable(Ranges, 1, Visibility);
		}

		void InitAsDescriptorTable(const D3D12_DESCRIPTOR_RANGE* Ranges, uint32 RangeCount, D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL)
		{
			RootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			RootParam.ShaderVisibility = Visibility;
			RootParam.DescriptorTable.NumDescriptorRanges = RangeCount;
			RootParam.DescriptorTable.pDescriptorRanges = Ranges;
		}

		const D3D12_ROOT_PARAMETER& operator() (void) const { return RootParam; }

	protected:

		void SetTableRange(D3D12_DESCRIPTOR_RANGE* Ranges, D3D12_DESCRIPTOR_RANGE_TYPE Type, uint32 Register, uint32 Count, uint32 Space = 0)
		{
			Ranges->RangeType = Type;
			Ranges->NumDescriptors = Count;
			Ranges->BaseShaderRegister = Register;
			Ranges->RegisterSpace = Space;
			Ranges->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		}

	protected:
		D3D12_ROOT_PARAMETER RootParam;

		friend class FRootSignature;
	};

	////////////////////////////////////////////////////////////////

	class FRootSignatureDx12 : public FShaderBinding
	{
	public:
		FRootSignatureDx12(uint32 NumRootParams = 0, uint32 NumStaticSamplers = 0) 
			: FShaderBinding(NumRootParams)
			, Finalized(false)
		{
			Reset(NumRootParams, NumStaticSamplers);
		}

		virtual ~FRootSignatureDx12()
		{
			TI_ASSERT(IsRenderThread());
			Signature = nullptr;
		}

		void Finalize(FRHI * RHI);
		
		FRootParameterDx12& GetParameter(uint32 EntryIndex)
		{
			return ParamArray[EntryIndex];
		}

		const FRootParameterDx12& GetParameter(uint32 EntryIndex) const
		{
			return ParamArray[EntryIndex];
		}

		void InitStaticSampler(uint32 Register, const D3D12_SAMPLER_DESC& NonStaticSamplerDesc,
			D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL);

		void InitStaticSampler(uint32 Register, const D3D12_STATIC_SAMPLER_DESC& InStaticSamplerDesc);

		void Finalize(ID3D12Device* D3dDevice, D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

		ID3D12RootSignature* Get() const
		{
			return Signature.Get(); 
		}
	private:
		void Reset(uint32 NumRootParams, uint32 NumStaticSamplers = 0)
		{
			if (NumRootParams > 0)
			{
				ParamArray.resize(NumRootParams);
			}

			if (NumStaticSamplers > 0)
			{
				SamplerArray.resize(NumStaticSamplers);
			}
			NumInitializedStaticSamplers = 0;
		}

	private:
		bool Finalized;
		uint32 NumInitializedStaticSamplers;
		TVector<FRootParameterDx12> ParamArray;
		TVector<D3D12_STATIC_SAMPLER_DESC> SamplerArray;

		ComPtr<ID3D12RootSignature> Signature;

		friend class FRHIDx12;
	};
}
#endif	// COMPILE_WITH_RHI_DX12