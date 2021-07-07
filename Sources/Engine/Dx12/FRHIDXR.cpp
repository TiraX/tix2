/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDXR.h"

#if COMPILE_WITH_RHI_DX12
#include "FRHIDx12Conversion.h"
#include "FMeshBufferDx12.h"

namespace tix
{
	FRHIDXR::FRHIDXR()
	{
	}

	bool FRHIDXR::Init(ComPtr<ID3D12Device> D3DDevice)
	{
		return SUCCEEDED(D3DDevice->QueryInterface(IID_PPV_ARGS(&DXRDevice)));
	}

	void FRHIDXR::AddBottomLevelAccelerationStructure(FMeshBufferDx12 * MeshBufferDx12)
	{
		D3D12_RAYTRACING_GEOMETRY_DESC GeometryDesc = {};
		BuildGeometryDesc(MeshBufferDx12, GeometryDesc);

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
		ComputePreBuildInfo(GeometryDesc, PrebuildInfo);
		AllocateASResource(PrebuildInfo, MeshBufferDx12->GetResourceName());
	}

	void FRHIDXR::BuildGeometryDesc(FMeshBufferDx12* MeshBufferDx12, D3D12_RAYTRACING_GEOMETRY_DESC& GeometryDesc)
	{
		GeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
		GeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

		GeometryDesc.Triangles.IndexFormat = GetDxIndexFormat(MeshBufferDx12->GetIndexType());
		GeometryDesc.Triangles.IndexBuffer = MeshBufferDx12->IndexBuffer.GetResource()->GetGPUVirtualAddress();
		GeometryDesc.Triangles.IndexCount = MeshBufferDx12->GetIndicesCount();

		GeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;	// Position always be RGB32F
		GeometryDesc.Triangles.VertexBuffer.StartAddress = MeshBufferDx12->VertexBuffer.GetResource()->GetGPUVirtualAddress();
		GeometryDesc.Triangles.VertexBuffer.StrideInBytes = MeshBufferDx12->GetStride();
		GeometryDesc.Triangles.VertexCount = MeshBufferDx12->GetVerticesCount();

		GeometryDesc.Triangles.Transform3x4 = NULL;
	}

	void FRHIDXR::ComputePreBuildInfo(const D3D12_RAYTRACING_GEOMETRY_DESC& GeometryDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo)
	{
		// Get the size requirements for the scratch and AS buffers.
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BottomLevelBuildDesc = {};
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BottomLevelInputs = BottomLevelBuildDesc.Inputs;
		BottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
		BottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		BottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
		BottomLevelInputs.NumDescs = 1;
		BottomLevelInputs.pGeometryDescs = &GeometryDesc;

		DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&BottomLevelInputs, &PrebuildInfo);
		TI_ASSERT(PrebuildInfo.ResultDataMaxSizeInBytes > 0);
	}

	void FRHIDXR::AllocateASResource(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo, const TString& Name)
	{
		D3D12_RESOURCE_STATES InitialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

		ComPtr<ID3D12Resource> AccelerationStructure;
		auto UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		auto BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(PrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
		VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
			&UploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&BufferDesc,
			InitialResourceState,
			nullptr,
			IID_PPV_ARGS(&AccelerationStructure)));
		DX_SETNAME(AccelerationStructure.Get(), Name + "-BLAS");
		BLAccelerationStructures.push_back(AccelerationStructure);
	}

	void FRHIDXR::BuildAllAccelerationStructures()
	{

	}
}
#endif	// COMPILE_WITH_RHI_DX12