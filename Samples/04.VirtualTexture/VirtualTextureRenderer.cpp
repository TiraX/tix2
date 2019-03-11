/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "VirtualTextureRenderer.h"

FVirtualTextureRenderer::FVirtualTextureRenderer()
{
}

FVirtualTextureRenderer::~FVirtualTextureRenderer()
{
}

void FVirtualTextureRenderer::InitInRenderThread()
{
	//FRHI * RHI = FRHI::Get();

}

void FVirtualTextureRenderer::Render(FRHI* RHI, FScene* Scene)
{
	Scene->PrepareViewUniforms();


    RHI->BeginRenderToFrameBuffer();
	// Render triangles to screen

}
