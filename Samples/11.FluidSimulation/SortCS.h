/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FSortCS : public FComputeTask
{
public:
	FSortCS();
	virtual ~FSortCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI* RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InPositions,
		FUniformBufferPtr InVelocities,
		FUniformBufferPtr InNumInCell,
		FUniformBufferPtr InCellParticleOffsets,
		FUniformBufferPtr InCellParticles,
		FUniformBufferPtr InSortedPositions,
		FUniformBufferPtr InSortedVelocities
		);

private:
	enum
	{
		SRV_POSITIONS,
		SRV_VELOCITIES,
		SRV_NUM_IN_CELL,
		SRV_CELL_PARTICLE_OFFSETS,
		SRV_CELL_PARTICLES,

		UAV_SORTED_POSITIONS,
		UAV_SORTED_VELOCITIES,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_PbfParams;
	FUniformBufferPtr UBRef_BoundInfo;

};
typedef TI_INTRUSIVE_PTR(FSortCS) FSortCSPtr;
