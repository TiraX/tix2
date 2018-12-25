/*
 TiX Engine v2.0 Copyright (C) 2018
 By ZhaoShuai tirax.cn@gmail.com
 */

#import <QuartzCore/QuartzCore.h>
#import "FMetalView.h"
#include "FRHIConfig.h"
#include "FRHIMetalConversion.h"

#if COMPILE_WITH_RHI_METAL
static FMetalView * s_view = nil;

@interface FMetalView (Private)
- (BOOL) setupSurface;
@end

@implementation FMetalView

+ (Class) layerClass
{
    return [CAMetalLayer class];
}

+ (id) viewWithFrame:(CGRect)frame
{
    return [[self alloc] initWithFrame:frame];
}

+ (id) sharedMetalView
{
    return s_view;
}

- (id) initWithFrame:(CGRect)frame
{
    TI_ASSERT(s_view == nil);
    if((self = [super initWithFrame:frame]))
    {
        self.MtlDevice = nil;
        if( ![self setupSurface] )
        {
            return nil;
        }
    }
    else
    {
        return nil;
    }
    
    s_view = self;
    return self;
}

-(BOOL) setupSurface
{
    self.opaque  = YES;
    self.backgroundColor = nil;
    
    // Init metal layer and create default Metal Device
    CAMetalLayer * MetalLayer = (CAMetalLayer *)self.layer;
    self.MtlDevice = MTLCreateSystemDefaultDevice();
    MetalLayer.device = self.MtlDevice;
    MetalLayer.pixelFormat = GetMetalPixelFormat(FRHIConfig::DefaultBackBufferFormat);
    MetalLayer.framebufferOnly = YES;
    
    return YES;
}

- (void) dealloc
{
    s_view = nil;
}

- (void) layoutSubviews
{
    [super layoutSubviews];
}

// Pass the touches to the superview
#pragma mark FMetalView - Touch Delegate

struct TouchTrack
{
    UITouch* m_touchPointer;
    
    TouchTrack()
    {
        m_touchPointer = NULL;
    }
};

const int32 MAX_TOUCHES = 5;
TouchTrack g_touchTracker[MAX_TOUCHES];

int32 GetFingerTrackIDByTouch(UITouch* touch)
{
    for (int32 i=0; i < MAX_TOUCHES; i++)
    {
        if (g_touchTracker[i].m_touchPointer == touch)
        {
            return i;
        }
    }
    
    return -1;
}

int32 AddNewTouch(UITouch* touch)
{
    for (int32 i=0; i < MAX_TOUCHES; i++)
    {
        if (!g_touchTracker[i].m_touchPointer)
        {
            g_touchTracker[i].m_touchPointer = touch;
            return i;
        }
    }
    return -1;
}

int32 GetTouchesActive()
{
    int32 count = 0;
    
    for (int32 i=0; i < MAX_TOUCHES; i++)
    {
        if (g_touchTracker[i].m_touchPointer)
        {
            count++;
        }
    }
    return count;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    long long time_stamp;
    time_stamp = TTimer::GetCurrentTimeMillis();
    TInput* input = TEngine::Get()->GetDevice()->GetInput();
    const int32 count = (int32)[touches count];
    
    for (UITouch *touch in touches)
    {
        int32 fingerID = GetFingerTrackIDByTouch(touch);
        
        if (fingerID == -1)
        {
            fingerID = AddNewTouch(touch);
        }
        else
        {
            continue;
        }
        CGPoint tpt = [touch locationInView:[touch view]];
        float force = 0.f;//[touch force];
        
        if ([[UIDevice currentDevice]systemVersion].floatValue >= 9.0)
            force = [touch force];
        
        input->PutEvent(EET_LEFT_DOWN, fingerID, time_stamp, force, 0, tpt.x * self.contentScaleFactor, tpt.y * self.contentScaleFactor);
        input->IncreaseInputCount(count);
    }
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    long long time_stamp;
    time_stamp = TTimer::GetCurrentTimeMillis();
    TInput* input = TEngine::Get()->GetDevice()->GetInput();
    //const int32 count = (int32)[touches count];
    
    for (UITouch *touch in touches)
    {
        int32 fingerID = GetFingerTrackIDByTouch(touch);
        if (fingerID != -1)
        {
            //found it
        }
        else
        {
            continue;
        }
        float force = 0.f;//[touch force];
        
        if ([[UIDevice currentDevice]systemVersion].floatValue >= 9.0)
            force = [touch force];

        CGPoint tpt = [touch locationInView:[touch view]];
        
        input->PutEvent(EET_MOVE, fingerID, time_stamp, force, 0, tpt.x * self.contentScaleFactor, tpt.y * self.contentScaleFactor);
    }
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    long long time_stamp;
    time_stamp = TTimer::GetCurrentTimeMillis();
    TInput* input = TEngine::Get()->GetDevice()->GetInput();
    const int32 count = (int32)[touches count];
    
    for (UITouch *touch in touches)
    {
        int32 fingerID = GetFingerTrackIDByTouch(touch);
        if (fingerID != -1)
        {
            g_touchTracker[fingerID].m_touchPointer = NULL; //clear it
        }
        else
        {
            //wasn't on our list
            continue;
        }
        
        CGPoint tpt = [touch locationInView:[touch view]];
        float force = 0.f;//[touch force];
        
        if ([[UIDevice currentDevice]systemVersion].floatValue >= 9.0)
            force = [touch force];

        input->PutEvent(EET_LEFT_UP, fingerID,time_stamp, force , 0, tpt.x * self.contentScaleFactor, tpt.y * self.contentScaleFactor );
    }
    input->DecreaseInputCount(count);
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    long long time_stamp;
    time_stamp = TTimer::GetCurrentTimeMillis();
    TInput* input = TEngine::Get()->GetDevice()->GetInput();
    const int32 count = (int32)[touches count];
    
    for (UITouch *touch in touches)
    {
        int32 fingerID = GetFingerTrackIDByTouch(touch);
        if (fingerID != -1)
        {
            g_touchTracker[fingerID].m_touchPointer = NULL; //clear it
        }
        else
        {
            continue;
        }
        CGPoint tpt = [touch locationInView:[touch view]];
        float force = 0.f;//[touch force];
        
        if ([[UIDevice currentDevice]systemVersion].floatValue >= 9.0)
            force = [touch force];

        input->PutEvent(EET_LEFT_UP, 0, time_stamp, force, 0, tpt.x * self.contentScaleFactor, tpt.y * self.contentScaleFactor);
    }
    input->DecreaseInputCount(count);
}

@end

#endif //COMPILE_WITH_RHI_METAL
