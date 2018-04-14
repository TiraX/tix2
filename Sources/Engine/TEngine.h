/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TEngine
	{
	public:      
        
		TI_API static TEngine* Get();
		TI_API static void	Create(int w, int h, void* handle, const char* windowName, uint32 option);
		TI_API static void	Destroy();
				
			   static void	InitComponent();
			   static void	InitGraphics(void* param);

		//TI_API TiDevice*	CreateDevice(int w, int h, void* handle, const char* name);
	private:
		TEngine();
		~TEngine();
		static TEngine* s_engine;

	protected:
	};
}
