/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMACGrid.h"
#include "FLIPSimRenderer.h"

TMACGrid::TMACGrid()
{
}

TMACGrid::~TMACGrid()
{
}

void TMACGrid::InitSize(const vector3di& InSize, float InSeperation)
{
	Size = InSize;
	Seperation = InSeperation;

	int32 TotalCount = Size.X * Size.Y * Size.Z;
	U.resize(TotalCount);
	V.resize(TotalCount);
	W.resize(TotalCount);
}