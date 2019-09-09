//
//  S_TileDetermination.metal
//  VirtualTexture
//
//  Created by Tirax on 2019/8/13.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

#include "Common.h"

static constant uint vt_mips[7] = {64, 32, 16, 8, 4, 2, 1};
static constant int vt_mips_offset[7] = { 0, 4096, 5120, 5376, 5440, 5456, 5460 };

#if VT_USE_TILESHADER
kernel void
S_TileDeterminationCS(imageblock<VTBufferData, imageblock_layout_implicit> SceneColor,
                      device float * OutputUV [[buffer(0)]],
                      ushort2 thread_local_position [[ thread_position_in_threadgroup ]]
                      )
{
    VTBufferData VTData = SceneColor.read(thread_local_position);
    
    half4 result = VTData.uv;
    //ScreenUV.read(pos);
    if (!isnan(result.z) && result.z >= 0.f)
    {
        int mip_level = int(result.z);
        uint vt_mip_size = vt_mips[mip_level];
        uint2 page = uint2(result.xy * vt_mip_size);
        int output_index = page.y * vt_mip_size + page.x + vt_mips_offset[mip_level];
        if (output_index < 5461)
            OutputUV[output_index] = 1.f;
    }
}

#else

kernel void
S_TileDeterminationCS(texture2d<half, access::read> ScreenUV [[texture(0)]],
                      device float * OutputUV [[buffer(0)]],
                      uint2 gid [[thread_position_in_grid]]
                      )
{
    // Check if the pixel is within the bounds of the output texture
    uint2 pos = gid * 4;
    if((pos.x >= ScreenUV.get_width()) || (pos.y >= ScreenUV.get_height()))
    {
        // Return early if the pixel is out of bounds
        return;
    }
    
    half4 result = ScreenUV.read(pos);
    if (!isnan(result.z) && result.z >= 0.f)
    {
        uint mip_level = uint(result.z);
        uint vt_mip_size = vt_mips[mip_level];
        uint2 page = uint2(result.xy * vt_mip_size);
        uint output_index = page.y * vt_mip_size + page.x + vt_mips_offset[mip_level];
        if (output_index < 5461)
            OutputUV[output_index] = 1.f;
    }
}

#endif
