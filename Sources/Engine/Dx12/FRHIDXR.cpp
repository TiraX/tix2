/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDXR.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FRHIDXR::FRHIDXR()
	{
	}

	bool FRHIDXR::Init(ComPtr<ID3D12Device> D3DDevice)
	{
		return SUCCEEDED(D3DDevice->QueryInterface(IID_PPV_ARGS(&DXRDevice)));
	}
}
#endif	// COMPILE_WITH_RHI_DX12