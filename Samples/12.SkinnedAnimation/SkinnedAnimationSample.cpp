// GTAOSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "SkinnedAnimationTicker.h"
#include "SkinnedAnimationRenderer.h"

int main()
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(-1);
	{
		TEngineDesc Desc;
		Desc.Name = "Skinned Animation";
		Desc.Width = 1600;
		Desc.Height = 900;

		TEngine::InitEngine(Desc);

		// Setup scenes
		TSkinnedAnimationTicker::SetupScene();

		// before tick and render
		TEngine::Get()->AddTicker(ti_new TSkinnedAnimationTicker());
		TEngine::Get()->AssignRenderer(ti_new FSkinnedAnimationRenderer());

		// Start Loop
		TEngine::Get()->Start();
	}
    return 0;
}

