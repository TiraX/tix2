/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "ComputeUniforms.h"

class FSceneMetaInfos
{
public:
	FSceneMetaInfos();
	~FSceneMetaInfos();

	void PrepareSceneResources(FRHI* RHI, FScene * Scene, FGPUCommandSignaturePtr CommandSignature);

	bool IsInited() const
	{
		return Inited;
	}
	FInstanceBufferPtr GetMergedInstanceBuffer()
	{
		return MergedInstanceBuffer;
	}
	FMeshBufferPtr GetMergedSceneMeshBuffer()
	{
		return MergedMeshBuffer;
	}
	FGPUCommandBufferPtr GetGPUCommandBuffer()
	{
		return GPUCommandBuffer;
	}
	FMeshBufferPtr GetMergedOccludeMeshBuffer()
	{
		return MergedOccludeMeshBuffer;
	}
	FGPUCommandBufferPtr GetGPUOccludeCommandBuffer()
	{
		return GPUOccludeCommandBuffer;
	}
	FScenePrimitiveBBoxesPtr GetPrimitiveBBoxesUniform()
	{
		return PrimitiveBBoxesUniform;
	}
	FSceneInstanceMetaInfoPtr GetInstanceMetaInfoUniform()
	{
		return InstanceMetaInfoUniform;
	}
private:


private:
	bool Inited;
	FInstanceBufferPtr MergedInstanceBuffer;

	FMeshBufferPtr MergedMeshBuffer;
	FGPUCommandBufferPtr GPUCommandBuffer;

	FMeshBufferPtr MergedOccludeMeshBuffer;
	FGPUCommandBufferPtr GPUOccludeCommandBuffer;

	FScenePrimitiveBBoxesPtr PrimitiveBBoxesUniform;
	FSceneInstanceMetaInfoPtr InstanceMetaInfoUniform;

	friend class FScene;
};