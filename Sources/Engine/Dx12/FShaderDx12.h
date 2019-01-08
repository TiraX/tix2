/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	class FShaderDx12 : public FShader
	{
	public:
		FShaderDx12(const TString& InShaderName);
		virtual ~FShaderDx12();

		void ReleaseShaderCode();
	protected:

	protected:
		TStream Shader;

		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
