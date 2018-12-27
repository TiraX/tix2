//
//  AppDelegate.m
//  SSSSSample
//
//  Created by Tirax on 2018/12/23.
//  Copyright © 2018 zhaoshuai. All rights reserved.
//

#import "AppDelegate.h"
#include "SSSSTicker.h"
#include "SSSSRenderer.h"

@interface AppDelegate ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Init TiX Engine
    CGRect bounds = [[UIScreen mainScreen] bounds];
    
    TEngineDesc Desc;
    Desc.Name = "SSSS Sample App";
    Desc.Width = bounds.size.width;
    Desc.Height = bounds.size.height;
    
    TEngine::InitEngine(Desc);
    
    // before tick and render
    TEngine::Get()->AddTicker(ti_new TSSSSTicker());
    //TEngine::Get()->AddRenderer(ti_new FSSSSRenderer());
    TEngine::Get()->AddRenderer(ti_new FS4TempRenderer());
    
    // Setup scenes
    //TSSSSTicker::SetupScene();
    
    // start tick and render
    
    // Start Loop
    TEngine::Get()->Start();
    return YES;
}


- (void)applicationWillResignActive:(UIApplication *)application {
    NSLog(@"TiX: applicationWillResignActive");
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    NSLog(@"TiX: applicationDidEnterBackground");
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    NSLog(@"TiX: applicationWillEnterForeground");
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    NSLog(@"TiX: applicationDidBecomeActive");
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    NSLog(@"TiX: applicationWillTerminate");
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}


@end
