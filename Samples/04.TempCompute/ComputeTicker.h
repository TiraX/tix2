/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TComputeTicker : public TTicker
{
public:
	TComputeTicker();
	virtual ~TComputeTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};