/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FApplyDeltaPosCS : public FComputeTask
{
public:
	FApplyDeltaPosCS();
	virtual ~FApplyDeltaPosCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InDeltaPosition,
		FUniformBufferPtr InPositions
		);

private:
	enum
	{
		SRV_DELTA_POS,
		UAV_POSITIONS,

		PARAM_TOTAL_COUNT,
	};

private:
	int32 ThreadsCount;
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_PbfParams;
	FUniformBufferPtr UBRef_BoundInfo;

};
typedef TI_INTRUSIVE_PTR(FApplyDeltaPosCS) FApplyDeltaPosCSPtr;
