/*
 TiX Engine v2.0 Copyright (C) 2018
 By ZhaoShuai tirax.cn@gmail.com
 */

#pragma once

#ifdef TI_PLATFORM_IOS

#import <UIKit/UIKit.h>

@class TViewController;
namespace tix
{
	class TDeviceIOS : public TDevice
	{
	public:
		TDeviceIOS(int32 w, int32 h);
		virtual ~TDeviceIOS();
        
		virtual bool Run();
		virtual void Resize(int w, int h);
        virtual int32 GetPreferredLanguage();
        void Show();
        
        TViewController* GetViewController()
        {
            return ViewController;
        }
        UIWindow* GetWindow()
        {
            return Window;
        }
	protected:
        void Create();
        void FindDeviceType();
        
	protected:
        UIWindow* Window;
        TViewController* ViewController;
	};
}

#endif //TI_PLATFORM_IOS
