/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TDeviceWin32.h"


#ifdef TI_PLATFORM_WIN32

static tix::TDeviceWin32* s_win32device = nullptr;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif
#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif
	if (!s_win32device)
		return DefWindowProc(hWnd, message, wParam, lParam);

	TInput* input = s_win32device->GetInput();
	TI_ASSERT(input);

	static int ClickCount=0;
	if (GetCapture() != hWnd && ClickCount > 0)
		ClickCount = 0;

	int x, y;
	switch (message)
	{
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
		}
		return 0;

	case WM_ERASEBKGND:
		return 0;

	case WM_SETCURSOR:
		break;

	case WM_MOUSEWHEEL:
		{
			short dis = (short)(HIWORD(wParam));
			input->PutEvent(EET_WHEEL, 0, TTimer::GetCurrentTimeMillis(), 0, 0, dis, 0);
			if (dis > 0)
			{
				input->PutEvent(EET_ZOOMIN, 0, TTimer::GetCurrentTimeMillis(), 0, 0, dis, 0);
			}
			else
			{
				input->PutEvent(EET_ZOOMOUT, 0, TTimer::GetCurrentTimeMillis(), 0, 0, -dis, 0);
			}
		}
		break;

	case WM_RBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		input->PutEvent(EET_RIGHT_DOWN, 0, TTimer::GetCurrentTimeMillis(), 0,0, x, y);
		return 0;

	case WM_RBUTTONUP:
		ClickCount--;
		if (ClickCount<1)
		{
			ClickCount=0;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			ReleaseCapture();
			input->PutEvent(EET_RIGHT_UP, 0, TTimer::GetCurrentTimeMillis(),0, 0, x, y);
		}
		return 0;

	case WM_LBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		input->PutEvent(EET_LEFT_DOWN, 0, TTimer::GetCurrentTimeMillis(),0, 0, x, y);
		return 0;

	case WM_LBUTTONUP:
		ClickCount--;
		if (ClickCount<1)
		{
			ClickCount=0;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			ReleaseCapture();
			input->PutEvent(EET_LEFT_UP, 0, TTimer::GetCurrentTimeMillis(),0, 0, x, y);
		}
		return 0;

	case WM_MBUTTONDOWN:
		ClickCount++;
		SetCapture(hWnd);
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		input->PutEvent(EET_MIDDLE_DOWN, 0, TTimer::GetCurrentTimeMillis(),0, 0, x, y);
		return 0;

	case WM_MBUTTONUP:
		ClickCount--;
		if (ClickCount<1)
		{
			ClickCount=0;
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			ReleaseCapture();
			input->PutEvent(EET_MIDDLE_UP, 0, TTimer::GetCurrentTimeMillis(),0, 0, x, y);
		}
		return 0;

	case WM_MOUSEMOVE:
		x = LOWORD(lParam);
		y = HIWORD(lParam);
		if (ClickCount > 0)
			input->PutEvent(EET_MOVE, 0, TTimer::GetCurrentTimeMillis(),0, 0, x, y);
		return 0;

	case WM_KEYDOWN:
		input->PutEvent(EET_KEY_DOWN, 0, TTimer::GetCurrentTimeMillis(), 0, uint32(wParam), 0, 0);
		if (wParam == VK_SPACE)
		{
			input->IncreaseInputCount(1);
		}
		return 0;
	case WM_KEYUP:
		input->PutEvent(EET_KEY_UP, 0, TTimer::GetCurrentTimeMillis(),0, uint32(wParam), 0, 0);
		if (wParam == VK_SPACE)
		{
			input->ResetInputCount();
		}
		return 0;

	case WM_SIZE:
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xFFF0) == SC_SCREENSAVE ||
			(wParam & 0xFFF0) == SC_MONITORPOWER)
			return 0;
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


namespace tix
{
	TDeviceWin32::TDeviceWin32(int w, int h, void* handle, const char* windowName)
		: TDevice(w, h)
		, HWnd((HWND)handle)
	{
		Create(windowName);
		s_win32device	= this;
		ExternalWindow	= (handle != NULL);
	}

	TDeviceWin32::~TDeviceWin32()
	{
		s_win32device = NULL;
	}

	void TDeviceWin32::Create(const char* windowName)
	{
		TString winversion;
		GetWindowsVersion(winversion);
		_LOG("%s\n", winversion.c_str());

		// create window
		TEngine* engine	= TEngine::Get();

		HINSTANCE hInstance = GetModuleHandle(0);

		// create the window, only if we do not use the null device
		if (HWnd == 0)
		{
			// Register Class
			WNDCLASSEX wcex;
			wcex.cbSize		= sizeof(WNDCLASSEX);
			wcex.style		= CS_HREDRAW | CS_VREDRAW;
			wcex.lpfnWndProc	= (WNDPROC)WndProc;
			wcex.cbClsExtra		= 0;
			wcex.cbWndExtra		= 0;
			wcex.hInstance		= hInstance;
			wcex.hIcon		= NULL;
			wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
			wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
			wcex.lpszMenuName	= 0;
			wcex.lpszClassName	= windowName;
			wcex.hIconSm		= 0;

			RegisterClassEx(&wcex);

			// calculate client size

			RECT clientSize;
			clientSize.top = 0;
			clientSize.left = 0;
			clientSize.right = Width;
			clientSize.bottom = Height;

			DWORD style = WS_POPUP;

			style = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

			AdjustWindowRect(&clientSize, style, FALSE);

			int realWidth = clientSize.right - clientSize.left;
			int realHeight = clientSize.bottom - clientSize.top;

			int windowLeft = (GetSystemMetrics(SM_CXSCREEN) - realWidth) / 2;
			int windowTop = (GetSystemMetrics(SM_CYSCREEN) - realHeight) / 2;

			// create window
			HWnd = CreateWindow( windowName, windowName, style, windowLeft, windowTop,
				realWidth, realHeight,	NULL, NULL, hInstance, NULL);
			TI_ASSERT(HWnd);
			if (!HWnd)
			{
				_LOG("Create window failed.\n");
				return ;
			}

			ShowWindow(HWnd , SW_SHOW);
			UpdateWindow(HWnd);

			MoveWindow(HWnd, windowLeft, windowTop, realWidth, realHeight, TRUE);
		}
		else
		{
			RECT r;
			GetWindowRect(HWnd, &r);
			Width = r.right - r.left;
			Height = r.bottom - r.top;
		}

		// set this as active window
		SetActiveWindow(HWnd);
		SetForegroundWindow(HWnd);
	}

	void TDeviceWin32::Resize(int w, int h)
	{
		TDevice::Resize(w, h);
		if (!ExternalWindow)
		{
			TI_ASSERT(HWnd != NULL);
			RECT r;
			GetWindowRect(HWnd, &r);
			MoveWindow(HWnd, r.left, r.top, w, h, TRUE);
		}
	}

	//! runs the device. Returns false if device wants to be deleted
	bool TDeviceWin32::Run()
	{
		MSG msg;

		bool quit = false;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);

			if (msg.hwnd == HWnd)
				WndProc(HWnd, msg.message, msg.wParam, msg.lParam);
			else
				DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
				quit = true;
		}

		return !quit;
	}

	void TDeviceWin32::GetWindowsVersion(TString& out)
	{
	}
}

#endif //TI_PLATFORM_WIN32