/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
#include <dxgi1_6.h>
#include <d3d12.h>
#include "d3dx12.h"
#include "FAccelerationStructure.h"

namespace tix
{
	class FBottomLevelAccelerationStructureDx12 : public FBottomLevelAccelerationStructure
	{
	public:
		FBottomLevelAccelerationStructureDx12();
		virtual ~FBottomLevelAccelerationStructureDx12();

		virtual void AddMeshBuffer(FMeshBufferPtr InMeshBuffer) override;
		virtual void Build() override;
	private:
		ComPtr<ID3D12Resource> AccelerationStructure;
		ComPtr<ID3D12Resource> ScratchResource;

		TVector<D3D12_RAYTRACING_GEOMETRY_DESC> GeometryDescs;
	};

	class FTopLevelAccelerationStructureDx12 : public FTopLevelAccelerationStructure
	{
	public:
		FTopLevelAccelerationStructureDx12() {};
		virtual ~FTopLevelAccelerationStructureDx12() {};

	private:
		ComPtr<ID3D12Resource> AccelerationStructure;
	};
}
#endif	// COMPILE_WITH_RHI_DX12