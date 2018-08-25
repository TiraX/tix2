/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHIDx12.h"
#include "FUniformBufferDx12.h"

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	FUniformBufferDx12::FUniformBufferDx12()
		: FUniformBuffer(ERF_Dx12)
		, CbvDescriptor(uint32(-1))
	{
	}

	FUniformBufferDx12::~FUniformBufferDx12()
	{
		Destroy();
	}

	void FUniformBufferDx12::Destroy()
	{
		TI_ASSERT(IsRenderThread());
		ConstantBuffer = nullptr;
		if (CbvDescriptor != uint32(-1))
		{
			FRHIDx12 * RHIDx12 = static_cast<FRHIDx12*>(FRHI::Get());
			RHIDx12->RecallDescriptor(EHT_CBV_SRV_UAV, CbvDescriptor);
			CbvDescriptor = uint32(-1);
		}
	}
}

#endif	// COMPILE_WITH_RHI_DX12