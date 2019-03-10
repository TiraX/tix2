/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API FComputeTask : public IReferenceCounted
	{
	public:
		FComputeTask(const TString& ComputeShaderName);
		virtual ~FComputeTask();

		void Finalize();
		virtual void Run(FRHI * RHI) = 0;

	protected:
		virtual void FinalizeInRenderThread();

	protected:
		TString ShaderName;
		FShaderPtr ComputeShader;
		FPipelinePtr ComputePipeline;
	};
}
