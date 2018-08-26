/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SSSSTicker.h"

TSSSSTicker::TSSSSTicker()
{
}

TSSSSTicker::~TSSSSTicker()
{
}

void TSSSSTicker::Tick(float Dt)
{
	TScene * Scene = TEngine::Get()->GetScene();
	Scene->UpdateAllNodesTransforms();
}