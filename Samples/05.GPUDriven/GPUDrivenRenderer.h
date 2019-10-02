/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FGPUDrivenRenderer : public FDefaultRenderer
{
public:
	FGPUDrivenRenderer();
	virtual ~FGPUDrivenRenderer();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	void UpdateGPUCommandBuffer(FRHI* RHI, FScene * Scene);
	void DrawGPUCommandBuffer(FRHI * RHI);

private:
	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;
	FArgumentBufferPtr AB_Result;

	FPipelinePtr DebugPipeline;
	FGPUCommandSignaturePtr GPUCommandSignature;
	FGPUCommandBufferPtr GPUCommandBuffer;
};
