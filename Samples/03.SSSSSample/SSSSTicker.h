/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TSSSSTicker : public TTicker
{
public:
	TSSSSTicker();
	virtual ~TSSSSTicker();

	virtual void Tick(float Dt) override;

	static void SetupScene();

protected:
};