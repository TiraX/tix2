// GTAOSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FluidSimTicker.h"
#include "FluidSimRenderer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(-1);
	{
		TEngineDesc Desc;
		Desc.Name = "Fluids Simulation with PBF Solver";
		Desc.Width = 1600;
		Desc.Height = 900;

		TEngine::InitEngine(Desc);

		// Setup scenes
		TFluidSimTicker::SetupScene();

		// before tick and render
		TEngine::Get()->AddTicker(ti_new TFluidSimTicker());
		TEngine::Get()->AssignRenderer(ti_new FFluidSimRenderer());

		// Start Loop
		TEngine::Get()->Start();
	}
    return 0;
}

