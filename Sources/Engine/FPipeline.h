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
			return ShaderBinding;
		}
	protected:

	protected:
		FShaderBindingPtr ShaderBinding;
	};
}
