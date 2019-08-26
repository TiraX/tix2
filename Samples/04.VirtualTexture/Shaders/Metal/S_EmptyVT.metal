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

vertex VSOutput S_EmptyVTVS(VertexInput vsInput [[ stage_in ]],
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

#if !(VT_ENABLED)

typedef struct FragmentShaderArguments {
    texture2d<half> IndirectTexture  [[ id(0) ]];
    texture2d<half> PhysicTexture  [[ id(1) ]];
} VTArguments;

#endif

fragment half4 S_EmptyVTPS(VSOutput input [[stage_in]],
                           constant VTArguments & fragmentArgs [[ buffer(PBIndex_MaterialInstance) ]])
{
    return half4(1,1,1,1);
}
