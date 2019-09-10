// SSSSSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "GPUDrivenTicker.h"
#include "GPUDrivenRenderer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(-1);
	{
		TEngineDesc Desc;
		Desc.Name = "GPU Driven App";
		Desc.Width = 1600;
		Desc.Height = 900;

		TEngine::InitEngine(Desc);

		// before tick and render
		TEngine::Get()->AddTicker(ti_new TGPUDrivenTicker());
		TEngine::Get()->AddRenderer(ti_new FGPUDrivenRenderer());

		// Setup scenes
		TGPUDrivenTicker::SetupScene();

		// start tick and render

		// Start Loop
		TEngine::Get()->Start();
	}
    return 0;
}

