/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderThread;

	enum E_Platform
	{
		EP_Unknown,
		EP_Windows,
		EP_Mac,
		EP_IOS,
		EP_Android,
	};

	class TEngine
	{
	public:      
        
		TI_API static TEngine* Get();
		TI_API static void	InitEngine(const TEngineDesc& Config);
		TI_API static void	Destroy();

		static E_Platform GetPlatform()
		{
			return CurrentPlatform;
		}

		TI_API void Start();
		TI_API TDevice*	GetDevice();

		TScene* GetScene()
		{
			return Scene;
		}

		TI_API void AddDefaultRenderer();
		TI_API void AddRenderer(FRenderer* Renderer);
		TI_API void AddTicker(TTicker* Ticker);

        TI_API void LowMemoryWarning() {};
        
#if defined (TI_PLATFORM_IOS)
        void TickIOS();
#endif

	private:
		TEngine();
		~TEngine();
		static TEngine* s_engine;

	protected:
		// Init every thing for engine
		void Init(const TEngineDesc& Config);
		void Tick();
		void TickFinished();

		void BeginFrame();
		void EndFrame();

	private:
		static E_Platform CurrentPlatform;

		TDevice * Device;

		TScene * Scene;
		TResourceLibrary * ResourceLibrary;

		uint64 LastFrameTime;
		TVector<TTicker*> Tickers;
	};
}
