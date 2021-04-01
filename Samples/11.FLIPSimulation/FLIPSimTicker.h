/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TFLIPSimTicker : public TTicker
{
public:
	TFLIPSimTicker();
	virtual ~TFLIPSimTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};