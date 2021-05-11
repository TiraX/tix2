/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TSkinnedAnimationTicker : public TTicker
{
public:
	TSkinnedAnimationTicker();
	virtual ~TSkinnedAnimationTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};