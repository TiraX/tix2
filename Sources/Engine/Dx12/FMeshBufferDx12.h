/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
#include "FGPUResourceDx12.h"

namespace tix
{
	class FMeshBufferDx12 : public FMeshBuffer
	{
	public:
		FMeshBufferDx12();
		FMeshBufferDx12(
			E_PRIMITIVE_TYPE InPrimType,
			uint32 InVSFormat,
			uint32 InVertexCount,
			E_INDEX_TYPE InIndexType,
			uint32 InIndexCount
		);
		virtual ~FMeshBufferDx12();
	protected:

	private:
		FGPUResourceDx12 VertexBuffer;
		FGPUResourceDx12 IndexBuffer;

		D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
		D3D12_INDEX_BUFFER_VIEW IndexBufferView;

		friend class FRHIDx12;
		friend class FGPUCommandBufferDx12;
	};

	/////////////////////////////////////////////////////////////

	class FInstanceBufferDx12 : public FInstanceBuffer
	{
	public:
		FInstanceBufferDx12();
		FInstanceBufferDx12(uint32 TotalInstancesCount, uint32 InstanceStride);
		virtual ~FInstanceBufferDx12();
	protected:

	private:
		FGPUResourceDx12 InstanceBuffer;
		D3D12_VERTEX_BUFFER_VIEW InstanceBufferView;

		friend class FRHIDx12;
		friend class FGPUCommandBufferDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
