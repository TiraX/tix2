/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FFLIPSimRenderer : public FDefaultRenderer
{
public:
	static const int32 FFT_Size = 512;

	FFLIPSimRenderer();
	virtual ~FFLIPSimRenderer();

	static FFLIPSimRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:

private:
	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;
};
