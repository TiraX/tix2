//
//  S_TileDetermination.metal
//  VirtualTexture
//
//  Created by Tirax on 2019/8/13.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;




typedef struct FragmentShaderArguments {
    texture2d<half, access::read> TexBaseColor  [[ id(0) ]];
    device float * Uniform [[ id(1) ]];
} ComputeShaderArguments;

//constant half3 kRec709Luma = half3(0.2126, 0.7152, 0.0722);
// Grayscale compute kernel
kernel void
S_TileDeterminationCS(
                    device ComputeShaderArguments & ComputeArgs [[ buffer(0) ]],
                    uint2                          gid         [[thread_position_in_grid]])
{
//    // Check if the pixel is within the bounds of the output texture
//    if((gid.x >= outTexture.get_width()) || (gid.y >= outTexture.get_height()))
//    {
//        // Return early if the pixel is out of bounds
//        return;
//    }
//
//    half4 inColor  = inTexture.read(gid);
//    half  gray     = dot(inColor.rgb, kRec709Luma);
//    outTexture.write(half4(gray, gray, gray, 1.0), gid);
}
