/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct FShaderValue
	{
		union 
		{
			FUniformBufferPtr UniformBuffer;
			FTexturePtr Texture;
			FRenderResourceTablePtr Table;
		};
	};

	class FRHI;
	class FShaderParamValue : public IReferenceCounted
	{
	public:
		FShaderParamValue(FShaderBindingPtr InBinding);
		~FShaderParamValue();

		void SetParamValue(uint32 InBindingIndex, FUniformBufferPtr UniformBuffer);
		void SetParamValue(uint32 InBindingIndex, FTexturePtr Texture);
		void SetParamValue(uint32 InBindingIndex, FRenderResourceTablePtr RenderResourceTable);

	private:
		FShaderBindingPtr ShaderBinding;
		TVector<FShaderValue> ShaderValues;
	};
}