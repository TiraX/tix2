/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SSSSRenderer.h"

FSSSSRenderer::FSSSSRenderer()
{
}

FSSSSRenderer::~FSSSSRenderer()
{
	RTTest = nullptr;
}

void FSSSSRenderer::InitInRenderThread()
{
	// Render target test case
	RTTest = FRenderTarget::Create(1280, 720);
	RTTest->AddColorBuffer(EPF_RGBA8, ERTC_COLOR0);
	RTTest->Compile();
}