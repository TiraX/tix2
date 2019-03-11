/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TVirtualTextureTicker : public TTicker
{
public:
	TVirtualTextureTicker();
	virtual ~TVirtualTextureTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};