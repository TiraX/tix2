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
		TDevice(int w, int h);
		virtual ~TDevice();

		virtual bool Run() = 0;

		int		GetWidth()
		{
			return Width;
		}

		int		GetHeight()
		{
			return Height;
		}

		TInput* GetInput()
		{
			return Input;
		}
		virtual void	Resize(int w, int h);

	protected:
		int			Width;
		int			Height;

		TInput*		Input;
	};
}