/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TDevice
	{
	public:
		static TDevice* CreateDevice(const TString& Name, int Width, int Height);
		static void DestoryDevice(TDevice* Device);

		virtual bool Run() = 0;

		vector2di GetDeviceSize()
		{
			return vector2di(Width, Height);
		}

		int GetWidth()
		{
			return Width;
		}

		int GetHeight()
		{
			return Height;
		}

		TInput* GetInput()
		{
			return Input;
		}
		virtual void	Resize(int w, int h);

	protected:
		TDevice(int w, int h);
		virtual ~TDevice();

	protected:
		int			Width;
		int			Height;

		TInput*		Input;
	};
}