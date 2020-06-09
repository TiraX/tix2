/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FShaderBinding.h"

namespace tix
{
	//struct FShaderValue
	//{
	//	FRenderResourcePtr RenderResource;
	//	int32 BindingType;

	//	FShaderValue()
	//		: BindingType(BINDING_TYPE_INVALID)
	//	{}

	//	FShaderValue(FRenderResourcePtr InResource, int32 InType)
	//		: RenderResource(InResource)
	//		, BindingType(InType)
	//	{}

	//	~FShaderValue()
	//	{
	//		RenderResource = nullptr;
	//	}
	//};

	//class FRHI;
	//class FShaderParamValue : public IReferenceCounted
	//{
	//public:
	//	FShaderParamValue(FShaderBindingPtr InBinding);
	//	~FShaderParamValue();

	//	void SetParamValue(uint32 InBindingIndex, FUniformBufferPtr UniformBuffer);
	//	void SetParamValue(uint32 InBindingIndex, FTexturePtr Texture);
	//	void SetParamValue(uint32 InBindingIndex, FRenderResourceTablePtr RenderResourceTable);

	//	void ApplyParamValues(FRHI * RHI);

	//private:
	//	FShaderBindingPtr ShaderBinding;
	//	TVector<FShaderValue> ShaderValues;
	//};
}