/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FDeltaPosCS : public FComputeTask
{
public:
	FDeltaPosCS();
	virtual ~FDeltaPosCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InNeighborNum,
		FUniformBufferPtr InNeighborParticles,
		FUniformBufferPtr InLambdas,
		FUniformBufferPtr InPositions
		);

private:
	enum
	{
		SRV_NEIGHBOR_NUM,
		SRV_NEIGHBOR_PARTICLES,
		SRV_LAMBDAS,

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
typedef TI_INTRUSIVE_PTR(FDeltaPosCS) FDeltaPosCSPtr;
