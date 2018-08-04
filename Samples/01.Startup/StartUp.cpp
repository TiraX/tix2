// StartUp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "TiX.h"

class MyTestThread : public TThread
{
public:
	MyTestThread(const TString& Name)
		: TThread(Name)
	{}

	virtual void Run()
	{
		static int32 a = 0;
		printf("MyTestThread says hello ... %d\n", a++);
		TThread::ThreadSleep(10);
	}
};

void TestNormalRun()
{
	// Start Loop
	TEngine::Get()->Start();
}

void TestThread()
{
	// Test thread
	MyTestThread* tt = ti_new MyTestThread("TestTixThreadddd");
	tt->Start();

	// Start Loop
	TEngine::Get()->Start();

	// When quit
	tt->Stop();
	ti_delete tt;
}
class MyTestTaskThread : public TTaskThread
{
public:
	MyTestTaskThread(const TString& Name)
		: TTaskThread(Name)
	{}
};

class MyTestTask : public TTask
{
	virtual void Execute()
	{
		static int32 aa = 0;
		_LOG(Log, "Task index: %d\n", aa++);
	}
};
void TestTask()
{
	// Test task thread
	MyTestTaskThread* tt = ti_new MyTestTaskThread("TestTaskThread");
	tt->Start();

	for (int32 i = 0; i < 4; ++i)
	{
		MyTestTask* task1 = ti_new MyTestTask;
		tt->AddTask(task1);
	}

	// Start Loop
	TEngine::Get()->Start();

	// When quit
	tt->Stop();
	ti_delete tt;
}

#include <sstream>
class TMyTicker : public TTicker
{
public:
	virtual void Tick(float Dt)
	{
		_LOG(Log, "TMyTestTicker is Ticking, Dt is %f.\n", Dt);
		TEngine::Get()->GetScene()->TickAllNodes(Dt);
	}
};

class FMyTestRenderer : public FRenderer
{
public:
	virtual void Render(FRHI * RHI, FScene * Scene) override
	{
		_LOG(Log, "My Test Renderer is rendering.\n");
		FRenderer::Render(RHI, Scene);
	}
};

void TestTickerAndRenderer()
{
	TEngine::Get()->AddTicker(ti_new TMyTicker());
	TEngine::Get()->AddRenderer(ti_new FMyTestRenderer());

	// Start Loop
	TEngine::Get()->Start();
}

// Pipeline do not have TResourceLibrary to hold reference, we need another global var to hold ref
TPipelinePtr GlobalPipelineTest = nullptr;
void TestTixResfileLoading()
{
	//return;
	const TString TargetMeshRes = "../../Content/head.tix";
	TResourcePtr meshbuffer = TResourceLibrary::Get()->LoadResource(TargetMeshRes);

	const TString TargetTextureRes = "../../Content/DiffuseMap.tix";
	TResourcePtr texture = TResourceLibrary::Get()->LoadResource(TargetTextureRes);

	const TString TargetVS = "../../Content/SampleVertexShader.cso";
	const TString TargetPS = "../../Content/SamplePixelShader.cso";
	GlobalPipelineTest = TPipeline::CreatePipeline(TargetVS, TargetPS, (EVSSEG_POSITION | EVSSEG_NORMAL | EVSSEG_TEXCOORD0 ));

	meshbuffer = nullptr;
	texture = nullptr;
}

void TestRenderMesh()
{
	TestTixResfileLoading();

	const TString TargetMeshRes = "../../Content/head.tix";
	TResourcePtr MeshRes = TResourceLibrary::Get()->LoadResource(TargetMeshRes);
	TMeshBufferPtr MeshBuffer = static_cast<TMeshBuffer*>(MeshRes.get());

	const TString TargetTextureRes = "../../Content/DiffuseMap.tix";
	TResourcePtr TextureRes = TResourceLibrary::Get()->LoadResource(TargetTextureRes);
	TTexturePtr Texture = static_cast<TTexture*>(TextureRes.get());

	// Add these to TScene
	TEngine::Get()->GetScene()->AddMeshToScene(MeshBuffer, GlobalPipelineTest, 0);
}

int main()
{
	{
		TEngineConfiguration Config;
		Config.Name = "First TiX Test App";
		Config.Width = 1280;
		Config.Height = 720;

		TEngine::InitEngine(Config);

		// before tick and render
		TestRenderMesh();

		// start tick and render
		//TestNormalRun();
		//TestThread();
		//TestTask();
		TestTickerAndRenderer();

		GlobalPipelineTest = nullptr;
	}

	_CrtDumpMemoryLeaks();

    return 0;
}

