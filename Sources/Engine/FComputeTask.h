/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FComputeTask : public IReferenceCounted
	{
	public:
		FComputeTask(const TString& ComputeShaderName);
		virtual ~FComputeTask();

		virtual void Finalize();

		virtual void SetParameter(int32 Index, FUniformBufferPtr Uniform);
		virtual void SetParameter(int32 Index, FRenderResourceTablePtr ResourceTable);
	protected:
		virtual void FinalizeInRenderThread();

	protected:
		TString ShaderName;
		FShaderPtr ComputeShader;
		FPipelinePtr ComputePipeline;
		TVector<FRenderResourcePtr> Resources;
	};
}
