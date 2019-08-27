//
//  S_Grass.metal
//  VirtualTexture
//
//  Created by Tirax on 2019/8/11.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;

#include "Common.h"
#include "VS_Instanced.h"
#include "PS_VT.h"

vertex VSOutput S_TreeOpaqueVS(VertexInput vsInput [[ stage_in ]],
                               constant EB_View & EB_View [[ buffer(VBIndex_View) ]],
                               constant EB_Primitive & EB_Primitive [[ buffer(VBIndex_Primitive) ]]
                               )
{
    VSOutput vsOutput;
    
    float3 WorldPosition = GetWorldPosition(vsInput);
    vsOutput.position = EB_View.ViewProjection * float4(WorldPosition, 1.0);
    vsOutput.texcoord0 = GetTextureCoords(vsInput);
    
    vsOutput.normal = vsInput.normal * 2.0h - 1.0h;
    vsOutput.tangent = vsInput.tangent * 2.0h - 1.0h;
    vsOutput.view = half3(EB_View.ViewPos - WorldPosition);
    
    return vsOutput;
}

#if (VT_ENABLED)
fragment VTBufferData S_TreeOpaquePS(VSOutput input [[stage_in]],
                              constant VTArguments & EB_VTArgs [[ buffer(PBIndex_VirtualTexture) ]],
                              constant EB_Primitive & EB_Primitive [[ buffer(PBIndex_Primitive) ]])
{
    VTBufferData Data;
    half4 Color = GetBaseColor(input.texcoord0.xy,
                               EB_Primitive.VTUVTransform,
                               EB_VTArgs.IndirectTexture,
                               EB_VTArgs.PhysicTexture);
    
    
    Data.color = Color;
    Data.uv = GetVTTextureCoords(input.texcoord0.xy, EB_Primitive.VTUVTransform);
    
    return Data;
}

#else   // VT_ENABLED
typedef struct FragmentShaderArguments {
    device float4 * Uniform [[ id(0) ]];
    texture2d<half> TexBaseColor  [[ id(1) ]];
    texture2d<half> TexNormal  [[ id(2) ]];
} FragmentShaderArguments;

fragment half4 S_TreeOpaquePS(VSOutput input [[stage_in]],
                              constant FragmentShaderArguments & fragmentArgs [[ buffer(PBIndex_MaterialInstance) ]],
                              constant EB_Primitive & EB_Primitive [[ buffer(PBIndex_Primitive) ]])
{
    half4 Color = GetBaseColor(fragmentArgs.TexBaseColor, input.texcoord0.xy);
    
    return Color;
}
#endif  // VT_ENABLED

