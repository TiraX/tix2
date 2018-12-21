/*
 TiX Engine v2.0 Copyright (C) 2018
 By ZhaoShuai tirax.cn@gmail.com
 */

#import <QuartzCore/QuartzCore.h>
#import "FMetalView.h"

#if COMPILE_WITH_RENDERER_METAL
static FMetalView *view = nil;

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
    return view;
}

- (id) initWithFrame:(CGRect)frame
{
    if((self = [super initWithFrame:frame]))
    {
    }
    else
    {
        return nil;
    }
    
    view = self;
    return self;
}

-(BOOL) setupSurface
{
    self.opaque  = YES;
    self.backgroundColor = nil;
    
    CAMetalLayer *_metalLayer = (CAMetalLayer *)self.layer;
    
    _metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    
    // this is the default but if we wanted to perform compute on the final rendering layer we could set this to no
    _metalLayer.framebufferOnly = YES;

    return YES;
}

- (void) generateBuffers
{
    if (!isFramebufferInited)
    {
        // init metal
        CAMetalLayer* layer = (CAMetalLayer*)self.layer;
        TI_ASSERT(0);
        //TiRendererMetal* rendererMetal  = (TiRendererMetal*)TEngine::Get()->GetRenderer()->GetActiveRenderer();
        //rendererMetal->InitMetalWithLayer(layer);
        isFramebufferInited = YES;
    }
}

- (void) dealloc
{
    view = nil;
    [super dealloc];
}

- (void) layoutSubviews
{
    [super layoutSubviews];
}

// Pass the touches to the superview
#pragma mark FMetalView - Touch Delegate

struct TouchTrack
{
    void* m_touchPointer;
    
    TouchTrack()
    {
        m_touchPointer = NULL;
    }
};

const int MAX_TOUCHES = 5;
TouchTrack g_touchTracker[MAX_TOUCHES];

int GetFingerTrackIDByTouch(void* touch)
{
    for (int i=0; i < MAX_TOUCHES; i++)
    {
        if (g_touchTracker[i].m_touchPointer == touch)
        {
            return i;
        }
    }
    
    return -1;
}

int AddNewTouch(void* touch)
{
    for (int i=0; i < MAX_TOUCHES; i++)
    {
        if (!g_touchTracker[i].m_touchPointer)
        {
            g_touchTracker[i].m_touchPointer = touch;
            return i;
        }
    }
    return -1;
}

int GetTouchesActive()
{
    int count = 0;
    
    for (int i=0; i < MAX_TOUCHES; i++)
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
    TInput* input = TEngine::Get()->GetDevice(0)->GetInput();
    const int count = [touches count];
    
    for (UITouch *touch in touches)
    {
        int fingerID = GetFingerTrackIDByTouch(touch);
        
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
    TInput* input = TEngine::Get()->GetDevice(0)->GetInput();
    const int count = [touches count];
    
    for (UITouch *touch in touches)
    {
        int fingerID = GetFingerTrackIDByTouch(touch);
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
    TInput* input = TEngine::Get()->GetDevice(0)->GetInput();
    const int count = [touches count];
    
    for (UITouch *touch in touches)
    {
        int fingerID = GetFingerTrackIDByTouch(touch);
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
    TInput* input = TEngine::Get()->GetDevice(0)->GetInput();
    const int count = [touches count];
    
    for (UITouch *touch in touches)
    {
        int fingerID = GetFingerTrackIDByTouch(touch);
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

#endif //COMPILE_WITH_RENDERER_METAL
