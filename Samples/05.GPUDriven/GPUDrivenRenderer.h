/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SceneMetaInfos.h"
#include "GPUCameraFrustum.h"
#include "GPUTileFrustumCullCS.h"
#include "GPUInstanceFrustumCullCS.h"
#include "GPUCameraFrustum.h"
#include "CopyVisibleTileCommandBuffer.h"
#include "CopyVisibleInstances.h"

class FGPUDrivenRenderer : public FDefaultRenderer
{
public:
	FGPUDrivenRenderer();
	virtual ~FGPUDrivenRenderer();

	static FGPUDrivenRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

	void UpdateFrustumUniform(const SViewFrustum& Frustum);

private:
	void UpdateGPUCommandBuffer(FRHI* RHI, FScene * Scene);
	void DrawGPUCommandBuffer(FRHI * RHI, FGPUCommandBufferPtr InGPUCommandBuffer);

	void SimluateCopyVisibleInstances(FRHI* RHI, FScene * Scene);

private:
	FSceneMetaInfos * SceneMetaInfo;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FRenderTargetPtr RT_DepthOnly;
	FPipelinePtr DepthOnlyPipeline;
	FArgumentBufferPtr AB_Result;

	FGPUCommandSignaturePtr GPUCommandSignature;
	FGPUCommandBufferPtr GPUCommandBuffer;
	FGPUCommandBufferPtr GPUCommandBufferTest;
	FGPUCommandBufferPtr ProcessedGPUCommandBuffer;

	FGPUCommandSignaturePtr PreZGPUCommandSignature;
	FGPUCommandBufferPtr PreZGPUCommandBuffer;
	FGPUCommandBufferPtr ProcessedPreZGPUCommandBuffer;

	SViewFrustum Frustum;
	FCameraFrustumUniformPtr FrustumUniform;

	//FGPUTileFrustumCullCSPtr TileCullCS;
	FGPUInstanceFrustumCullCSPtr InstanceCullCS;

	//FCopyVisibleTileCommandBufferPtr CopyVisibleCommandBuffer;
	FCopyVisibleInstancesPtr CopyVisibleInstances;
};
