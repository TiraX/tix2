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
		FPipeline(FShaderPtr InShader);
		virtual ~FPipeline();

		FShaderPtr GetShader()
		{
			return Shader;
		}
	protected:

	protected:
		FShaderPtr Shader;
	};
}
