/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	class FShaderMetal : public FShader
	{
	public:
		FShaderMetal(const TString& InShaderName);
		virtual ~FShaderMetal();

	protected:

	protected:

		friend class FRHIMetal;
	};
}

#endif	// COMPILE_WITH_RHI_METAL
