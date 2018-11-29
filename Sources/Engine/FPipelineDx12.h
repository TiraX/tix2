/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	class FPipelineDx12 : public FPipeline
	{
	public:
		FPipelineDx12();
		virtual ~FPipelineDx12();
	protected:

	private:
		ComPtr<ID3D12PipelineState> PipelineState;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
