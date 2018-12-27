/*
 TiX Engine v2.0 Copyright (C) 2018
 By ZhaoShuai tirax.cn@gmail.com
 */

#import <Foundation/Foundation.h>
#import "TDirectorCaller.h"

static id s_sharedDirectorCaller = nil;

@interface NSObject(CADisplayLink)
+(id) displayLinkWithTarget: (id)arg1 selector:(SEL)arg2;
-(void) addToRunLoop: (id)arg1 forMode: (id)arg2;
-(void) setFrameInterval: (int)interval;
-(void) invalidate;
@end

@implementation TDirectorCaller

@synthesize interval;

+(id) sharedDirectorCaller
{
    if (s_sharedDirectorCaller == nil)
    {
        s_sharedDirectorCaller = [TDirectorCaller new];
    }
    
    return s_sharedDirectorCaller;
}

+(void) destroy
{
    s_sharedDirectorCaller = nil;
}

-(void) alloc
{
    interval = 1;
}

-(void) dealloc
{
    displayLink = nil;
}

-(void) startMainLoopWithInterval:(double) intervalNew
{
    // CCDirector::setAnimationInterval() is called, we should invalidate it first
    [displayLink invalidate];
    displayLink = nil;
    
    self.interval = 60.0f * intervalNew;
    
    displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:self selector:@selector(doCaller:)];
    [displayLink setFrameInterval: self.interval];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}
                      
-(void) doCaller: (id) sender
{
    NSLog(@"doCaller");
}

@end
