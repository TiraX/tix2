/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TSkinnedAnimationTicker : public TTicker, public TEventHandler
{
public:
	TSkinnedAnimationTicker();
	virtual ~TSkinnedAnimationTicker();

	virtual void Tick(float Dt) override;
	virtual bool OnEvent(const TEvent& e) override;

	static void SetupScene();

protected:
};