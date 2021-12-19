// GTAOSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "RTAOTicker.h"
#include "RTAORenderer.h"
#include "RTXTest.h"
#include "RTXTest2.h"

static const bool DoTest = true;
int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(-1);
	if (DoTest)
	{
		TRTXTest RTXTest;
		RTXTest.Run();
	}
	if (!DoTest)
	{
		TEngineDesc Desc;
		Desc.Name = "RTAO App";
		Desc.Width = 1600;
		Desc.Height = 900;

		TEngine::InitEngine(Desc);

		// Setup scenes
		TRTAOTicker::SetupScene();

		// before tick and render
		TEngine::Get()->AddTicker(ti_new TRTAOTicker());
		TEngine::Get()->AssignRenderer(ti_new FRTAORenderer());

		// start tick and render

		// Start Loop
		TEngine::Get()->Start();
	}
    return 0;
}

