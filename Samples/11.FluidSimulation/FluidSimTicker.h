/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TFluidSimTicker : public TTicker
{
public:
	TFluidSimTicker();
	virtual ~TFluidSimTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};