/*
 TiX Engine v2.0 Copyright (C) 2018
 By ZhaoShuai tirax.cn@gmail.com
 */

#import <UIKit/UIKit.h>
#import <QuartzCore/CAMetalLayer.h>
#import <Metal/Metal.h>

@interface FMetalView : UIView<UIGestureRecognizerDelegate>

@property (strong, nonatomic) CAMetalLayer * MtlLayer;
@property (strong, nonatomic) id <MTLDevice> MtlDevice;

// create View
+ (id) viewWithFrame:(CGRect)frame;

// get the view object
+(id) sharedMetalView;

@end
