/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TGTAOTicker : public TTicker
{
public:
	TGTAOTicker();
	virtual ~TGTAOTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};