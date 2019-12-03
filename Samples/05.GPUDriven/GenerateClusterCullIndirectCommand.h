/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SceneMetaInfos.h"

class FGenerateClusterCullIndirectCommand : public FComputeTask
{
public:
	FGenerateClusterCullIndirectCommand();
	virtual ~FGenerateClusterCullIndirectCommand();

	void PrepareResources(FRHI * RHI, FUniformBufferPtr InClusterQueue, FGPUCommandBufferPtr DispatchCommandBuffer);
	virtual void Run(FRHI * RHI) override;
private:

private:
	FUniformBufferPtr ClustersQueue;
	FGPUCommandBufferPtr CommandBuffer;
	FRenderResourceTablePtr ResourceTable;
};
typedef TI_INTRUSIVE_PTR(FGenerateClusterCullIndirectCommand) FGenerateClusterCullIndirectCommandPtr;
