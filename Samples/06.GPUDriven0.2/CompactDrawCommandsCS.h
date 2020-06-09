/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "ComputeUniforms.h"

class FCompactDrawCommandsCS : public FComputeTask
{
public:
	FCompactDrawCommandsCS();
	virtual ~FCompactDrawCommandsCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		FGPUCommandBufferPtr InCommandBuffer,
		FUniformBufferPtr InCounterResetBuffer
		);

	FGPUCommandBufferPtr GetCompactDrawCommands()
	{
		return CompactDrawCommands;
	}
private:
	enum
	{
		SRV_DRAW_COMMANDS,
		UAV_COMPACT_COMMANDS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	FUniformBufferPtr CommandsInfo;	// b0

	FGPUCommandBufferPtr DrawCommandBuffer;	// t0

	FGPUCommandBufferPtr CompactDrawCommands;	// u0

	FUniformBufferPtr CounterResetBuffer;

};
typedef TI_INTRUSIVE_PTR(FCompactDrawCommandsCS) FCompactDrawCommandsCSPtr;
