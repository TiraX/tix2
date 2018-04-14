/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#ifdef TI_PLATFORM_WIN32

namespace tix
{
	class TWin32Device : public TDevice
	{
	public:
		TWin32Device(int w, int h, void* handle, const char* windowName);
		virtual ~TWin32Device();

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