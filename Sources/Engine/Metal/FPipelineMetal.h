/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	class FPipelineMetal : public FPipeline
	{
	public:
		FPipelineMetal();
		virtual ~FPipelineMetal();
	protected:

	private:
		friend class FRHIMetal;
	};
}

#endif	// COMPILE_WITH_RHI_METAL
