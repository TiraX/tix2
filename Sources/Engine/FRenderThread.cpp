/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderThread.h"
#include "FRenderer.h"
#include "FRHI.h"

namespace tix
{
	FRenderThread::FRenderThread()
		: TThread("RenderThread")
		, Renderer(nullptr)
		, RHI(nullptr)
	{
	}

	FRenderThread::~FRenderThread()
	{
	}

	void FRenderThread::CreateRenderComponents()
	{
		// Create renderer to execute render pass
		Renderer = ti_new FRenderer();

		// Create RHI to submit commands to GPU
		RHI = FRHI::CreateRHI(ERHI_DX12);
	}

	void FRenderThread::DestroyRenderComponents()
	{
		SAFE_DELETE(Renderer);
		SAFE_DELETE(RHI);
	}

	void FRenderThread::Run()
	{

	}

	void FRenderThread::OnThreadStart()
	{
		CreateRenderComponents();
	}

	void FRenderThread::OnThreadEnd()
	{
		DestroyRenderComponents();
	}
}