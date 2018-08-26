/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FSSSSRenderer : public FDefaultRenderer
{
public:
	FSSSSRenderer();
	virtual ~FSSSSRenderer();

	virtual void InitInRenderThread() override;
private:
protected:
	FRenderTargetPtr RTTest;
};