/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
	FUniformBufferPtr GetMergedClusterMetaInfo()
	{
		return MergedClusterMetaInfo;
	}
	FGPUCommandBufferPtr GetGPUOccludeCommandBuffer()
	{
		return GPUOccludeCommandBuffer;
	}
	FSceneMeshBBoxesPtr GetSceneMeshBBoxesUniform()
	{
		return SceneMeshBBoxesUniform;
	}
	FSceneInstanceMetaInfoPtr GetInstanceMetaInfoUniform()
	{
		return InstanceMetaInfoUniform;
	}
	uint32 GetMaxInstanceClusterCount() const
	{
		return TotalInstanceClusters;
	}
	uint32 GetTotalTrianglesInScene() const
	{
		return TotalTrianglesInScene;
	}
private:


private:
	bool Inited;
	FInstanceBufferPtr MergedInstanceBuffer;

	FMeshBufferPtr MergedMeshBuffer;
	FGPUCommandBufferPtr GPUCommandBuffer;

	FMeshBufferPtr MergedOccludeMeshBuffer;
	FGPUCommandBufferPtr GPUOccludeCommandBuffer;

	FUniformBufferPtr MergedClusterMetaInfo;
	FSceneMeshBBoxesPtr SceneMeshBBoxesUniform;
	FSceneInstanceMetaInfoPtr InstanceMetaInfoUniform;

	uint32 TotalInstanceClusters;
	uint32 TotalTrianglesInScene;

	friend class FScene;
};