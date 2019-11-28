/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SceneMetaInfos.h"

//BEGIN_UNIFORM_BUFFER_STRUCT(FCopyCommandsParams)
//	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, Info)
//END_UNIFORM_BUFFER_STRUCT(FCopyCommandsParams)

class FGenerateClusterCullIndirectCommand : public FComputeTask
{
public:
	FGenerateClusterCullIndirectCommand();
	virtual ~FGenerateClusterCullIndirectCommand();

	void PrepareResources(FRHI * RHI);
	void UpdateComputeArguments(
		FRHI * RHI, 
		FSceneMetaInfos * SceneMetaInfos,
		FUniformBufferPtr InstanceVisibleInfo, 
		FUniformBufferPtr InstanceMetaInfo, 
		FGPUCommandBufferPtr GPUCommandBuffer,
		FGPUCommandBufferPtr ProcessedGPUCommandBuffer);
	virtual void Run(FRHI * RHI) override;
private:

private:
};
typedef TI_INTRUSIVE_PTR(FGenerateClusterCullIndirectCommand) FGenerateClusterCullIndirectCommandPtr;
