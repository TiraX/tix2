/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FApplyGravityCS : public FComputeTask
{
public:
	FApplyGravityCS();
	virtual ~FApplyGravityCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InPositions,
		FUniformBufferPtr InVelocities,
		FUniformBufferPtr InPosOld
		);

private:
	enum
	{
		SRV_VELOCITIES,

		UAV_POSITIONS,
		UAV_POS_OLD,

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
typedef TI_INTRUSIVE_PTR(FApplyGravityCS) FApplyGravityCSPtr;
