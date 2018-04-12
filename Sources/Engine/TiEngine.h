/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace ti
{
	class TiEngine
	{
	public:      
        
		TI_API static TiEngine* Get();
		TI_API static void	Create(int w, int h, void* handle, const char* windowName, uint32 option);
		TI_API static void	Destroy();
				
			   static void	InitComponent();
			   static void	InitGraphics(void* param);

		//TI_API TiDevice*	CreateDevice(int w, int h, void* handle, const char* name);
	private:
		TiEngine();
		~TiEngine();
		static TiEngine* s_engine;

	protected:
	};
}
