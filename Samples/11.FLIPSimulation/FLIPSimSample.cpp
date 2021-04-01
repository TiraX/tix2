// GTAOSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FLIPSimTicker.h"
#include "FLIPSimRenderer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(-1);
	{
		TEngineDesc Desc;
		Desc.Name = "Ocean App";
		Desc.Width = 1600;
		Desc.Height = 900;

		TEngine::InitEngine(Desc);

		// Setup scenes
		TFLIPSimTicker::SetupScene();

		// before tick and render
		TEngine::Get()->AddTicker(ti_new TFLIPSimTicker());
		TEngine::Get()->AssignRenderer(ti_new FFLIPSimRenderer());

		// start tick and render

		// Start Loop
		TEngine::Get()->Start();
	}
    return 0;
}

