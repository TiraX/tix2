/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SceneMetaInfos.h"
#include "HiZDownSampleCS.h"
#include "InstanceFrustumCullCS.h"
#include "ComputeDispatchCmd.h"
#include "ClusterCullCS.h"
#include "TriangleCullCS.h"
#include "CompactDrawCommandsCS.h"

class FGPUDrivenRenderer : public FDefaultRenderer
{
public:
	FGPUDrivenRenderer();
	virtual ~FGPUDrivenRenderer();

	static FGPUDrivenRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

	void UpdateFrustumUniform(const SViewFrustum& Frustum);

private:
	void TestDrawSceneIndirectCommandBuffer(
		FRHI * RHI, 
		FScene * Scene, 
		FMeshBufferPtr MeshBuffer, 
		FUniformBufferPtr CulledIndexBuffer,
		FInstanceBufferPtr InstanceBuffer,
		FGPUCommandSignaturePtr CommandSignature,
		FGPUCommandBufferPtr CommandBuffer
	);

private:
	FSceneMetaInfos * SceneMetaInfo;

	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FTexturePtr HiZTexture;

	FRenderTargetPtr RT_DepthOnly;
	FPipelinePtr DepthOnlyPipeline;

	FGPUCommandSignaturePtr GPUCommandSignature;
	FGPUCommandSignaturePtr GPUOccludeCommandSignature;
	FGPUCommandBufferPtr ProcessedGPUCommandBuffer;

	FGPUCommandSignaturePtr PreZGPUCommandSignature;
	FGPUCommandBufferPtr PreZGPUCommandBuffer;
	FGPUCommandBufferPtr ProcessedPreZGPUCommandBuffer;

	SViewFrustum Frustum;
	FCameraFrustumUniformPtr FrustumUniform;
	FOcclusionInfoPtr OcclusionInfo;

	// Compute Tasks
	FInstanceFrustumCullCSPtr InstanceFrustumCullCS;
	FHiZDownSampleCSPtr HiZDownSample;

	FComputeDispatchCmdCSPtr ClusterDispatchCmdCS;
	FClusterCullCSPtr ClusterCullCS;

	FTriangleCullCSPtr TriangleCullCS;
	FCompactDrawCommandsCSPtr CompactDCCS;
};
