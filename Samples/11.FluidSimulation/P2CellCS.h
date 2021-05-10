/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FP2CellCS : public FComputeTask
{
public:
	FP2CellCS();
	virtual ~FP2CellCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InPositions,
		FUniformBufferPtr InNumInCell,
		FUniformBufferPtr InCellParticles
		);

private:
	enum
	{
		SRV_POSITIONS,
		UAV_NUM_IN_CELL,
		UAV_CELL_PARTICLES,

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
typedef TI_INTRUSIVE_PTR(FP2CellCS) FP2CellCSPtr;
