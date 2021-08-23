/*
TiX Engine v2.0 Copyright (C) 2018~2021
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRtxPipeline : public FRenderResource
	{
	public:
		FRtxPipeline(FShaderPtr InShader);
		virtual ~FRtxPipeline();

		FShaderPtr GetShaderLib()
		{
			return ShaderLib;
		}
	protected:

	protected:
		FShaderPtr ShaderLib;
	};
}
