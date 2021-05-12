/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TFluidSimTicker : public TTicker, public TEventHandler
{
public:
	TFluidSimTicker();
	virtual ~TFluidSimTicker();

	virtual void Tick(float Dt) override;
	virtual bool OnEvent(const TEvent& e) override;

	static void SetupScene();

protected:
};