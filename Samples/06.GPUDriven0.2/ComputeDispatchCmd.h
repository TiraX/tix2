/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FComputeDispatchCmdCS : public FComputeTask
{
public:
	FComputeDispatchCmdCS();
	virtual ~FComputeDispatchCmdCS();

	void PrepareResources(FRHI * RHI, const TString& InName);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InVisibleInstanceCount
	);

	FUniformBufferPtr GetDispatchThreadCount()
	{
		return DispatchThreadCount;
	}

private:
	enum 
	{
		PARAM_DISPATCH_THREAD_COUNT,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	FUniformBufferPtr VisibleInstanceCount;	// b0

	FUniformBufferPtr DispatchThreadCount;	// u0
};
typedef TI_INTRUSIVE_PTR(FComputeDispatchCmdCS) FComputeDispatchCmdCSPtr;
