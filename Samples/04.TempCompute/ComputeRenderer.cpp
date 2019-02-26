/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ComputeRenderer.h"

FComputeRenderer::FComputeRenderer()
{
}

FComputeRenderer::~FComputeRenderer()
{
}

void FComputeRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
}

void FComputeRenderer::Render(FRHI* RHI, FScene* Scene)
{
	Scene->PrepareViewUniforms();
    RHI->BeginRenderToFrameBuffer();
}
