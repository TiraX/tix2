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
	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 ViewWidth = 1600;
	const int32 ViewHeight = TEngine::AppInfo.Height * ViewWidth / TEngine::AppInfo.Width;

	TStreamPtr ArgumentValues = ti_new TStream;
	TVector<FTexturePtr> ArgumentTextures;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(ViewWidth, ViewHeight);
#if defined (TIX_DEBUG)
	RT_BasePass->SetResourceName("BasePass");
#endif
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);

	if (FVTSystem::IsEnabled())
	{
		// Second for render uv onto it.
		RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR1, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	}
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->Compile();

	AB_Result = RHI->CreateArgumentBuffer(FSRender.GetFullScreenShader());
	{
		ArgumentValues->Reset();
		ArgumentTextures.clear();
		ArgumentTextures.push_back(RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResource(AB_Result, ArgumentValues, ArgumentTextures);
	}
}

void FVirtualTextureRenderer::Render(FRHI* RHI, FScene* Scene)
{
	// Render Base Pass
	RHI->PushRenderTarget(RT_BasePass, "BasePass");
	RenderDrawList(RHI, Scene, LIST_OPAQUE);
	RenderDrawList(RHI, Scene, LIST_MASK);
	RHI->PopRenderTarget();

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
