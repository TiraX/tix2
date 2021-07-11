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

	bool FRHIDXR::Init(ID3D12Device* D3DDevice, ID3D12GraphicsCommandList* D3DCommandList)
	{
		return SUCCEEDED(D3DDevice->QueryInterface(IID_PPV_ARGS(&DXRDevice))) &&
			SUCCEEDED(D3DCommandList->QueryInterface(IID_PPV_ARGS(&DXRCommandList)));
	}

	//void FRHIDXR::AddBottomLevelAccelerationStructure(FMeshBufferDx12 * MeshBufferDx12)
	//{
	//	D3D12_RAYTRACING_GEOMETRY_DESC GeometryDesc = {};
	//	BuildGeometryDesc(MeshBufferDx12, GeometryDesc);

	//	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
	//	ComputePreBuildInfo(GeometryDesc, PrebuildInfo);
	//	AllocateASResource(PrebuildInfo, MeshBufferDx12->GetResourceName());
	//}

	//void FRHIDXR::BuildGeometryDesc(FMeshBufferDx12* MeshBufferDx12, D3D12_RAYTRACING_GEOMETRY_DESC& GeometryDesc)
	//{
	//	GeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	//	GeometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	//	GeometryDesc.Triangles.IndexFormat = GetDxIndexFormat(MeshBufferDx12->GetIndexType());
	//	GeometryDesc.Triangles.IndexBuffer = MeshBufferDx12->IndexBuffer.GetResource()->GetGPUVirtualAddress();
	//	GeometryDesc.Triangles.IndexCount = MeshBufferDx12->GetIndicesCount();

	//	GeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;	// Position always be RGB32F
	//	GeometryDesc.Triangles.VertexBuffer.StartAddress = MeshBufferDx12->VertexBuffer.GetResource()->GetGPUVirtualAddress();
	//	GeometryDesc.Triangles.VertexBuffer.StrideInBytes = MeshBufferDx12->GetStride();
	//	GeometryDesc.Triangles.VertexCount = MeshBufferDx12->GetVerticesCount();

	//	GeometryDesc.Triangles.Transform3x4 = NULL;
	//}

	//void FRHIDXR::ComputePreBuildInfo(const D3D12_RAYTRACING_GEOMETRY_DESC& GeometryDesc, D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo)
	//{
	//	// Get the size requirements for the scratch and AS buffers.
	//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BottomLevelBuildDesc = {};
	//	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& BottomLevelInputs = BottomLevelBuildDesc.Inputs;
	//	BottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	//	BottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	//	BottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	//	BottomLevelInputs.NumDescs = 1;
	//	BottomLevelInputs.pGeometryDescs = &GeometryDesc;

	//	DXRDevice->GetRaytracingAccelerationStructurePrebuildInfo(&BottomLevelInputs, &PrebuildInfo);
	//	TI_ASSERT(PrebuildInfo.ResultDataMaxSizeInBytes > 0);
	//}

	//void FRHIDXR::AllocateASResource(const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo, const TString& Name)
	//{
	//	D3D12_RESOURCE_STATES InitialResourceState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;

	//	ComPtr<ID3D12Resource> AccelerationStructure;
	//	auto UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	//	auto BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(PrebuildInfo.ResultDataMaxSizeInBytes, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	//	VALIDATE_HRESULT(DXRDevice->CreateCommittedResource(
	//		&UploadHeapProperties,
	//		D3D12_HEAP_FLAG_NONE,
	//		&BufferDesc,
	//		InitialResourceState,
	//		nullptr,
	//		IID_PPV_ARGS(&AccelerationStructure)));
	//	DX_SETNAME(AccelerationStructure.Get(), Name + "-BLAS");
	//	BLAccelerationStructures.push_back(AccelerationStructure);
	//}

	//void FRHIDXR::BuildAllAccelerationStructures()
	//{
	//	// Build all bottom layer AS
	//	for (auto& bottomLevelASpair : m_vBottomLevelAS)
	//	{
	//		auto& bottomLevelAS = bottomLevelASpair.second;
	//		if (bForceBuild || bottomLevelAS.IsDirty())
	//		{
	//			ScopedTimer _prof(bottomLevelAS.GetName(), commandList);

	//			D3D12_GPU_VIRTUAL_ADDRESS baseGeometryTransformGpuAddress = 0;
	//			ThrowIfFalse(PrebuildInfo.ScratchDataSizeInBytes <= scratch->GetDesc().Width, L"Insufficient scratch buffer size provided!");

	//			if (baseGeometryTransformGPUAddress > 0)
	//			{
	//				UpdateGeometryDescsTransform(baseGeometryTransformGPUAddress);
	//			}

	//			currentID = (currentID + 1) % Sample::FrameCount;
	//			m_cacheGeometryDescs[currentID].clear();
	//			m_cacheGeometryDescs[currentID].resize(GeometryDescs.size());
	//			copy(GeometryDescs.begin(), GeometryDescs.end(), m_cacheGeometryDescs[currentID].begin());

	//			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {};
	//			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS& bottomLevelInputs = bottomLevelBuildDesc.Inputs;
	//			{
	//				bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	//				bottomLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	//				bottomLevelInputs.Flags = BuildFlags;
	//				if (m_isBuilt && m_allowUpdate && m_updateOnBuild)
	//				{
	//					bottomLevelInputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	//					bottomLevelBuildDesc.SourceAccelerationStructureData = AccelerationStructure->GetGPUVirtualAddress();
	//				}
	//				bottomLevelInputs.NumDescs = static_cast<UINT>(m_cacheGeometryDescs[currentID].size());
	//				bottomLevelInputs.pGeometryDescs = m_cacheGeometryDescs[currentID].data();

	//				bottomLevelBuildDesc.ScratchAccelerationStructureData = scratch->GetGPUVirtualAddress();
	//				bottomLevelBuildDesc.DestAccelerationStructureData = AccelerationStructure->GetGPUVirtualAddress();
	//			}

	//			commandList->SetDescriptorHeaps(1, &descriptorHeap);
	//			commandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, nullptr);

	//			m_isDirty = false;
	//			m_isBuilt = true;

	//			// Since a single scratch resource is reused, put a barrier in-between each call.
	//			// PEFORMANCE tip: use separate scratch memory per BLAS build to allow a GPU driver to overlap build calls.
	//			commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(bottomLevelAS.GetResource()));
	//		}
	//	}
	//}
}
#endif	// COMPILE_WITH_RHI_DX12