/*
 TiX Engine v2.0 Copyright (C) 2018
 By ZhaoShuai tirax.cn@gmail.com
 */

#pragma once

#import <Foundation/Foundation.h>

@interface TDirectorCaller : NSObject {
        id displayLink;
        int interval;
}
@property (readwrite) int interval;
-(void) startMainLoopWithInterval:(double)interval;
-(void) doCaller: (id) sender;
+(id) sharedDirectorCaller;
+(void) destroy;
@end
