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
	class FMeshBufferDx12;
	class FRHIDXR
	{
	private:
		friend class FRHIDx12;
		FRHIDXR();
		~FRHIDXR() {};

		bool Init(ComPtr<ID3D12Device> D3DDevice);
		void AddBottomLevelAccelerationStructure(FMeshBufferDx12* MeshBufferDx12);
		void BuildAllAccelerationStructures();

	private:
		void BuildGeometryDesc(FMeshBufferDx12* MeshBufferDx12, D3D12_RAYTRACING_GEOMETRY_DESC& GeometryDesc);
		void ComputePreBuildInfo(const D3D12_RAYTRACING_GEOMETRY_DESC& GeometryDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo);
		void AllocateASResource(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo, const TString& Name);

	private:
		// DirectX Raytracing (DXR) attributes
		ComPtr<ID3D12Device5> DXRDevice;
		ComPtr<ID3D12GraphicsCommandList4> DXRCommandList;

		TVector<ComPtr<ID3D12Resource>> BLAccelerationStructures;

	};
}
#endif	// COMPILE_WITH_RHI_DX12