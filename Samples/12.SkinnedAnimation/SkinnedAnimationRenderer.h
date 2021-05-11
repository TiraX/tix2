/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FSkinnedAnimationRenderer : public FDefaultRenderer
{
public:
	static const int32 FFT_Size = 512;

	FSkinnedAnimationRenderer();
	virtual ~FSkinnedAnimationRenderer();

	static FSkinnedAnimationRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:

private:
	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;
};
