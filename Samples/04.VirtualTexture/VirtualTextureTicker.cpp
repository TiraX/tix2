/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "VirtualTextureTicker.h"

TVirtualTextureTicker::TVirtualTextureTicker()
{
}

TVirtualTextureTicker::~TVirtualTextureTicker()
{
}

void TVirtualTextureTicker::Tick(float Dt)
{
	TScene * Scene = TEngine::Get()->GetScene();
	Scene->UpdateAllNodesTransforms();
}

void TVirtualTextureTicker::SetupScene()
{
}
