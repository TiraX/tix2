/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FUpdateVelocityCS : public FComputeTask
{
public:
	FUpdateVelocityCS();
	virtual ~FUpdateVelocityCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InPosOld,
		FUniformBufferPtr InPositions,
		FUniformBufferPtr InVelocities
		);

private:
	enum
	{
		SRV_POS_OLD,
		
		UAV_POSITIONS,
		UAV_VELOCITIES,

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
typedef TI_INTRUSIVE_PTR(FUpdateVelocityCS) FUpdateVelocityCSPtr;
