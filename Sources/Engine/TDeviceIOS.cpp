/*
 TiX Engine v2.0 Copyright (C) 2018
 By ZhaoShuai tirax.cn@gmail.com
 */

#include "stdafx.h"

#include "TDeviceIOS.h"
#import "TViewController.h"
#import "FMetalView.h"
#import <sys/utsname.h>

#ifdef TI_PLATFORM_IOS

namespace tix
{
	TIOSDevice::TIOSDevice(int32 w, int32 h)
        : TDevice(w, h)
        , Window(nil)
        , ViewController(nil)
	{
        Create();
        
        // change dir to app base dir
        NSString* path = [[NSBundle mainBundle] resourcePath];
        chdir([path UTF8String]);
        
        FindDeviceType();
	}
    
	TIOSDevice::~TIOSDevice()
	{
        Window = nil;
	}
    
	void TIOSDevice::Resize(int32 w, int32 h)
	{
		TDevice::Resize(w, h);
	}
    
	//! runs the device. Returns false if device wants to be deleted
	bool TIOSDevice::Run()
	{
        return true;
	}
    
    void TIOSDevice::FindDeviceType()
    {
        //struct utsname systemInfo;
        //uname(&systemInfo);
        
        //NSString* device = [NSString stringWithCString:systemInfo.machine encoding:NSUTF8StringEncoding];
    }
    
    int32 TIOSDevice::GetPreferredLanguage()
    {
        //float ios_version = [[UIDevice currentDevice].systemVersion floatValue];
        //NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
        //NSArray* languages = [NSLocale preferredLanguages];//[defs objectForKey:@"AppleLanguages"];
        //NSString* preferredLang = [languages objectAtIndex:0];
        
        //return TI_LANG_DEFAULT;
        return 0;
    }
    
    void TIOSDevice::Create()
    {
        FMetalView *_View = nil;
        Window = [[UIWindow alloc] initWithFrame:CGRectMake(0, 0, Width, Height)];
        // Override point for customization after application launch.
        _View = [FMetalView viewWithFrame: [Window bounds]];
        
        ViewController = [[TViewController alloc] initWithNibName:nil bundle:nil];
        ViewController.view = _View;
        
        // Set RootViewController to window
        [Window setRootViewController:ViewController];
        [_View setMultipleTouchEnabled:YES];
    }
    
    void TIOSDevice::Show()
    {
        TI_ASSERT(Window != nil);
        [Window makeKeyAndVisible];
        
        if ([FMetalView sharedMetalView]) {
            [[FMetalView sharedMetalView] generateBuffers];
        }
    }
}

#endif //TI_PLATFORM_IOS
