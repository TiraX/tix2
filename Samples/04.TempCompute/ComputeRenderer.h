/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FComputeRenderer : public FDefaultRenderer
{
public:
	FComputeRenderer();
	virtual ~FComputeRenderer();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
protected:
};
