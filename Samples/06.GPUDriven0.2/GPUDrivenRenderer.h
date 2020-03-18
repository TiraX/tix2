/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SceneMetaInfos.h"
#include "HiZDownSampleCS.h"
#include "InstanceFrustumCullCS.h"

class FGPUDrivenRenderer : public FDefaultRenderer
{
public:
	FGPUDrivenRenderer();
	virtual ~FGPUDrivenRenderer();

	static FGPUDrivenRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	void TestDrawSceneIndirectCommandBuffer(FRHI * RHI, FScene * Scene);

private:
	FSceneMetaInfos * SceneMetaInfo;

	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FGPUCommandSignaturePtr GPUCommandSignature;
	FGPUCommandBufferPtr ProcessedGPUCommandBuffer;

	FGPUCommandSignaturePtr PreZGPUCommandSignature;
	FGPUCommandBufferPtr PreZGPUCommandBuffer;
	FGPUCommandBufferPtr ProcessedPreZGPUCommandBuffer;

	SViewFrustum Frustum;

	// Compute Tasks
	FInstanceFrustumCullCSPtr InstanceFrustumCullCS;
};
