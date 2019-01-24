/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FPipeline : public FRenderResource
	{
	public:
		FPipeline();
		virtual ~FPipeline();

		void SetShaderBinding(FShaderBindingPtr Binding)
		{
			ShaderBinding = Binding;
		}

		FShaderBindingPtr GetShaderBinding()
		{
			TI_TODO("ShaderBinding is a dx12 only feature , should do for dx 12 only.");
			return ShaderBinding;
		}
	protected:

	protected:
		FShaderBindingPtr ShaderBinding;
	};
}
