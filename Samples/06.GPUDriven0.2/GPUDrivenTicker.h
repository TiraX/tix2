/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TGPUDrivenTicker : public TTicker
{
public:
	TGPUDrivenTicker();
	virtual ~TGPUDrivenTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};