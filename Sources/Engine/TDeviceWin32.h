/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#ifdef TI_PLATFORM_WIN32

namespace tix
{
	class TDeviceWin32 : public TDevice
	{
	public:
		TDeviceWin32(int w, int h, void* handle, const char* windowName);
		virtual ~TDeviceWin32();

		virtual bool Run();
		virtual void Resize(int w, int h);

		HWND	GetWnd()
		{
			return HWnd;
		}
	protected:
		void Create(const char* windowName);

		// get from irrlicht
		void GetWindowsVersion(TString& out);

	protected:
		HWND	HWnd;
		bool	ExternalWindow;
	};
}

#endif //TI_PLATFORM_WIN32