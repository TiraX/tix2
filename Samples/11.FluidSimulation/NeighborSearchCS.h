/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FNeighborSearchCS : public FComputeTask
{
public:
	FNeighborSearchCS();
	virtual ~FNeighborSearchCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InNumInCell,
		FUniformBufferPtr InCellParticleOffsets,
		FUniformBufferPtr InPositions,
		FUniformBufferPtr InNeighborNum,
		FUniformBufferPtr InNeighborParticles
		);

private:
	enum
	{
		SRV_NUM_IN_CELL,
		SRV_CELL_PARTICLE_OFFSETS,
		SRV_POSITIONS,

		UAV_NEIGHBOR_NUM,
		UAV_NEIGHBOR_PARTICLES,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_PbfParams;
	FUniformBufferPtr UBRef_BoundInfo;

};
typedef TI_INTRUSIVE_PTR(FNeighborSearchCS) FNeighborSearchCSPtr;
