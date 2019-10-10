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

class FGPUDrivenRenderer : public FDefaultRenderer
{
public:
	FGPUDrivenRenderer();
	virtual ~FGPUDrivenRenderer();

	static FGPUDrivenRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;
	virtual void EndRenderFrame(FScene* Scene) override;

	void UpdateFrustumUniform(const SViewFrustum& Frustum);

private:
	void UpdateGPUCommandBuffer(FRHI* RHI, FScene * Scene);
	void DrawGPUCommandBuffer(FRHI * RHI, FGPUCommandBufferPtr InGPUCommandBuffer);

private:
	FSceneMetaInfos * SceneMetaInfo;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;
	FArgumentBufferPtr AB_Result;

	FPipelinePtr DebugPipeline;
	FGPUCommandSignaturePtr GPUCommandSignature;
	FGPUCommandBufferPtr GPUCommandBuffer;
	FGPUCommandBufferPtr ProcessedGPUCommandBuffer;

	FCameraFrustumUniformPtr FrustumUniform;

	FGPUTileFrustumCullCSPtr TileCullCS;
	FGPUInstanceFrustumCullCSPtr InstanceCullCS;

	FCopyVisibleTileCommandBufferPtr CopyVisibleCommandBuffer;
};
