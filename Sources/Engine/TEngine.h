/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderThread;

	class TEngine
	{
	public:      
        
		TI_API static TEngine* Get();
		TI_API static void	InitEngine(const TEngineConfiguration& Config);
		TI_API static void	Destroy();

		TI_API void Start();
		TI_API TDevice*	GetDevice();

		TI_API void AddRenderer(FRenderer* Renderer);
		TI_API void AddTicker(TTicker* Ticker);
	private:
		TEngine();
		~TEngine();
		static TEngine* s_engine;

	protected:
		// Init every thing for engine
		void Init(const TEngineConfiguration& Config);
		void Tick();
		void TickFinished();

	private:
		TDevice * Device;

		uint64 LastFrameTime;
		TVector<TTicker*> Tickers;
	};
}
