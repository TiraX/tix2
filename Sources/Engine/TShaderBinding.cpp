/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	//TShaderBinding::TShaderBinding()
	//	: TResource(ERES_SHADER_BINDING)
	//{}

	//TShaderBinding::~TShaderBinding()
	//{
	//}

	//void TShaderBinding::InitRenderThreadResource()
	//{
	//	TI_ASSERT(ShaderBindingResource == nullptr);
	//	ShaderBindingResource = FRHI::Get()->CreateShaderBinding((uint32)Bindings.size());

	//	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TShaderBindingUpdateResource,
	//		FShaderBindingPtr, ShaderBindingResource, ShaderBindingResource,
	//		TVector<TBindingParamInfo>, BindingInfos, Bindings,
	//		{
	//			FRHI::Get()->UpdateHardwareResource(ShaderBindingResource, BindingInfos);
	//		});
	//}

	//void TShaderBinding::DestroyRenderThreadResource()
	//{
	//	if (ShaderBindingResource != nullptr)
	//	{
	//		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TShaderBindingDestroyResource,
	//			FShaderBindingPtr, ShaderBindingResource, ShaderBindingResource,
	//			{
	//				ShaderBindingResource = nullptr;
	//			});
	//		ShaderBindingResource = nullptr;
	//	}
	//}
}
