// SSSSSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ComputeTicker.h"
#include "ComputeRenderer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(-1);
	{
		TEngineDesc Desc;
		Desc.Name = "Temp Compute App";
		Desc.Width = 1600;
		Desc.Height = 900;

		TEngine::InitEngine(Desc);

		// before tick and render
		TEngine::Get()->AddTicker(ti_new TComputeTicker());
		TEngine::Get()->AddRenderer(ti_new FComputeRenderer());

		// Setup scenes
		TComputeTicker::SetupScene();

		// start tick and render

		// Start Loop
		TEngine::Get()->Start();
	}
    return 0;
}

