/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FLambdaCS : public FComputeTask
{
public:
	FLambdaCS();
	virtual ~FLambdaCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InPositions,
		FUniformBufferPtr InNeighborNum,
		FUniformBufferPtr InNeighborParticles,
		FUniformBufferPtr InLambdas
		);

private:
	enum
	{
		SRV_POSITIONS,
		SRV_NEIGHBOR_NUM,
		SRV_NEIGHBOR_PARTICLES,

		UAV_LAMBDAS,

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
typedef TI_INTRUSIVE_PTR(FLambdaCS) FLambdaCSPtr;
