/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FVirtualTextureRenderer : public FDefaultRenderer
{
public:
	FVirtualTextureRenderer();
	virtual ~FVirtualTextureRenderer();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;
	FArgumentBufferPtr AB_Result;

	FRenderTargetPtr RT_UVAnalysis;
};
