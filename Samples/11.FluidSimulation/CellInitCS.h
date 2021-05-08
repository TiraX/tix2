/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FCellInitCS : public FComputeTask
{
public:
	FCellInitCS();
	virtual ~FCellInitCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InPbfParams,
		FUniformBufferPtr InBoundInfo,
		FUniformBufferPtr InNumInCell,
		FUniformBufferPtr InCellParticleOffsets
		);

private:
	enum
	{
		UAV_NUM_IN_CELL,
		UAV_CELL_PARTICLE_OFFSETS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_PbfParams;
	FUniformBufferPtr UBRef_BoundInfo;

	// SRV & UAV
	FUniformBufferPtr UBRef_NumInCell;
	FUniformBufferPtr UBRef_CellParticleOffsets;

};
typedef TI_INTRUSIVE_PTR(FCellInitCS) FCellInitCSPtr;
