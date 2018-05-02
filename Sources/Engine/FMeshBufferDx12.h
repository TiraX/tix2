/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
#include "dx12/d3dx12.h"

namespace tix
{
	class FMeshBufferDx12 : public FMeshBuffer
	{
	public:
		FMeshBufferDx12();
		virtual ~FMeshBufferDx12();

	protected:
		virtual void CreateHardwareBuffer() override;

	private:
		void GetLayout(TVector<D3D12_INPUT_ELEMENT_DESC>& Layout);
	};
}

#endif	// COMPILE_WITH_RHI_DX12
