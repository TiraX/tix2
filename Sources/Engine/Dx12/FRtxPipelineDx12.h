/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	class FRtxPipelineDx12 : public FRtxPipeline
	{
	public:
		FRtxPipelineDx12(FShaderPtr InShader);
		virtual ~FRtxPipelineDx12();
	protected:

	private:
		ComPtr<ID3D12StateObject> StateObject;
		FUniformBufferPtr ShaderTable;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
