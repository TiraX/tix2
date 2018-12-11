/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class SeparableSSS;
class FSSSSRenderer : public FDefaultRenderer
{
public:
	FSSSSRenderer();
	virtual ~FSSSSRenderer();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
protected:
	FRenderTargetPtr RTBasePass;
	FFullScreenRender FSRender;
	FRenderResourceTablePtr RTTextureTable;

	SeparableSSS* S4Effect;
};