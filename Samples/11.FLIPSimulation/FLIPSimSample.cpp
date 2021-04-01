// GTAOSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "FLIPSimTicker.h"
#include "FLIPSimRenderer.h"
#include "TFlipSolver.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(-1);
	{
		TEngineDesc Desc;
		Desc.Name = "Flip Simulation";
		Desc.Width = 1600;
		Desc.Height = 900;

		TEngine::InitEngine(Desc);

		// Setup scenes
		TFLIPSimTicker::SetupScene();

		// before tick and render
		TEngine::Get()->AddTicker(ti_new TFLIPSimTicker());
		TEngine::Get()->AssignRenderer(ti_new FFLIPSimRenderer());

		{
			// Setup Simulation
			TFlipSolver Solver;
			Solver.InitGrid(vector3di(32, 64, 32), 0.1f);
			Solver.CreateParticlesInSphere(vector3df(1.6f, 3.2f, 1.6f), 1.f, 0.1f);

			// Do Simulation
			for (int i = 0; i < 120; ++i)
			{
				Solver.DoSimulation(0.033f);
			}
		}

		// Start Loop
		TEngine::Get()->Start();
	}
    return 0;
}

