//----------------------------------------------------------------------------------
// File:    HorizonBasedAOEngine.fx
// Authors: Louis Bavoil & Miguel Sainz
// Email:   sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//----------------------------------------------------------------------------------

Texture2D<float3> tRandom;
Texture2D<float>  tLinDepth;
Texture2D<float4> tNormal;

#define M_PI 3.14159265f

SamplerState samNearest
{
    Filter   = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

//----------------------------------------------------------------------------------
DepthStencilState DisableDepth
{
    DepthEnable    = FALSE;
    DepthWriteMask = ZERO;
};

BlendState DisableBlend
{
    BlendEnable[0] = false;
};

//----------------------------------------------------------------------------------
cbuffer cb0
{
    float2 g_Dirs[32];
}

cbuffer cb1
{
    float  g_NumSteps;
    float  g_NumDir;
    float  g_R;
    float  g_inv_R;
    float  g_sqr_R;
    float  g_AngleBias;
    float  g_TanAngleBias;
    float  g_Attenuation;
    float  g_Contrast;
    float  g_AspectRatio;
    float  g_InvAspectRatio;
    float2 g_FocalLen;
    float2 g_InvFocalLen;
    float2 g_InvResolution;
    float2 g_Resolution;
    float2 g_ZLinParams;
}

//----------------------------------------------------------------------------------
struct PostProc_VSOut
{
    float4 pos   : SV_Position;
    float2 tex   : TEXCOORD0;
    float2 texUV : TEXCOORD1;
};

// Vertex shader that generates a full screen quad with texcoords
// To use draw 3 vertices with primitive type triangle
PostProc_VSOut FullScreenQuadVS( uniform bool useAutoRadius, uint id : SV_VertexID )
{
    PostProc_VSOut output = (PostProc_VSOut)0.0f;
    float2 tex = float2( (id << 1) & 2, id & 2 );
    output.tex = tex * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f);
    output.pos = float4( output.tex, 0.0f, 1.0f );
    output.tex /= g_FocalLen;
    
    // Bottom left pixel is (0,0) and bottom right is (1,1)
    output.texUV = float2( (id << 1) & 2, id & 2 );
   
    return output;
}


//----------------------------------------------------------------------------------
float tangent(float3 P, float3 S)
{
    return (P.z - S.z) / length(S.xy - P.xy);
}

//----------------------------------------------------------------------------------
float3 uv_to_eye(float2 uv, float eye_z)
{
    uv = (uv * float2(2.0, -2.0) - float2(1.0, -1.0));
    return float3(uv * g_InvFocalLen * eye_z, eye_z);
}

//----------------------------------------------------------------------------------
float3 fetch_eye_pos(float2 uv)
{
    float z = tLinDepth.SampleLevel(samNearest, float3(uv, 0), 0);
    return uv_to_eye(uv, z);
}

//----------------------------------------------------------------------------------
float3 tangent_eye_pos(float2 uv, float4 tangentPlane)
{
    // view vector going through the surface point at uv
    float3 V = fetch_eye_pos(uv);
    float NdotV = dot(tangentPlane.xyz, V);
    // intersect with tangent plane except for silhouette edges
    if (NdotV < 0.0) V *= (tangentPlane.w / NdotV);
    return V;
}

float length2(float3 v) { return dot(v, v); } 

//----------------------------------------------------------------------------------
float3 min_diff(float3 P, float3 Pr, float3 Pl)
{
    float3 V1 = Pr - P;
    float3 V2 = P - Pl;
    return (length2(V1) < length2(V2)) ? V1 : V2;
}

//----------------------------------------------------------------------------------
float falloff(float r)
{
    return 1.0f - g_Attenuation*r*r;
}

//----------------------------------------------------------------------------------
float2 snap_uv_offset(float2 uv)
{
    return round(uv * g_Resolution) * g_InvResolution;
}

float2 snap_uv_coord(float2 uv)
{
    //return (floor(uv * g_Resolution) + 0.5f) * g_InvResolution;
    return uv - (frac(uv * g_Resolution) - 0.5f) * g_InvResolution;
}

//----------------------------------------------------------------------------------
float tan_to_sin(float x)
{
    return x / sqrt(1.0f + x*x);
}

//----------------------------------------------------------------------------------
float3 tangent_vector(float2 deltaUV, float3 dPdu, float3 dPdv)
{
    return deltaUV.x * dPdu + deltaUV.y * dPdv;
}

//----------------------------------------------------------------------------------
float tangent(float3 T)
{
    return -T.z / length(T.xy);
}

//----------------------------------------------------------------------------------
float biased_tangent(float3 T)
{
    float phi = atan(tangent(T)) + g_AngleBias;
    return tan(min(phi, M_PI*0.5));
}

//----------------------------------------------------------------------------------
void integrate_direction(inout float ao, float3 P, float2 uv, float2 deltaUV,
                         float numSteps, float tanH, float sinH)
{
    for (float j = 1; j <= numSteps; ++j) {
        uv += deltaUV;
        float3 S = fetch_eye_pos(uv);
        
        // Ignore any samples outside the radius of influence
        float d2  = length2(S - P);
        if (d2 < g_sqr_R) {
            float tanS = tangent(P, S);

            [branch]
            if(tanS > tanH) {
                // Accumulate AO between the horizon and the sample
                float sinS = tanS / sqrt(1.0f + tanS*tanS);
                float r = sqrt(d2) * g_inv_R;
                ao += falloff(r) * (sinS - sinH);
                
                // Update the current horizon angle
                tanH = tanS;
                sinH = sinS;
            }
        }
    }
}

//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion_LowQuality(float2 deltaUV, 
                                             float2 uv0, 
                                             float3 P, 
                                             float numSteps, 
                                             float randstep)
{
    // Randomize starting point within the first sample distance
    float2 uv = uv0 + snap_uv_offset( randstep * deltaUV );
    
    // Snap increments to pixels to avoid disparities between xy 
    // and z sample locations and sample along a line
    deltaUV = snap_uv_offset( deltaUV );

    float tanT = tan(-M_PI*0.5 + g_AngleBias);
    float sinT = (g_AngleBias != 0.0) ? tan_to_sin(tanT) : -1.0;

    float ao = 0;
    integrate_direction(ao, P, uv, deltaUV, numSteps, tanT, sinT);

    // Integrate opposite directions together
    deltaUV = -deltaUV;
    uv = uv0 + snap_uv_offset( randstep * deltaUV );
    integrate_direction(ao, P, uv, deltaUV, numSteps, tanT, sinT);

    // Divide by 2 because we have integrated 2 directions together
    // Subtract 1 and clamp to remove the part below the surface
    return max(ao * 0.5 - 1.0, 0.0);
}

//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion(float2 deltaUV, 
                                  float2 uv0, 
                                  float3 P, 
                                  float numSteps, 
                                  float randstep,
                                  float3 dPdu,
                                  float3 dPdv )
{
    // Randomize starting point within the first sample distance
    float2 uv = uv0 + snap_uv_offset( randstep * deltaUV );
    
    // Snap increments to pixels to avoid disparities between xy 
    // and z sample locations and sample along a line
    deltaUV = snap_uv_offset( deltaUV );

    // Compute tangent vector using the tangent plane
    float3 T = deltaUV.x * dPdu + deltaUV.y * dPdv;

    float tanH = biased_tangent(T);
    float sinH = tanH / sqrt(1.0f + tanH*tanH);

    float ao = 0;
    for(float j = 1; j <= numSteps; ++j) {
        uv += deltaUV;
        float3 S = fetch_eye_pos(uv);
        
        // Ignore any samples outside the radius of influence
        float d2  = length2(S - P);
        if (d2 < g_sqr_R) {
            float tanS = tangent(P, S);

            [branch]
            if(tanS > tanH) {
                // Accumulate AO between the horizon and the sample
                float sinS = tanS / sqrt(1.0f + tanS*tanS);
                float r = sqrt(d2) * g_inv_R;
                ao += falloff(r) * (sinS - sinH);
                
                // Update the current horizon angle
                tanH = tanS;
                sinH = sinS;
            }
        } 
    }

    return ao;
}

//----------------------------------------------------------------------------------
float AccumulatedHorizonOcclusion_Quality(float2 deltaUV, 
                                          float2 uv0, 
                                          float3 P, 
                                          float numSteps, 
                                          float randstep,
                                          float3 dPdu,
                                          float3 dPdv )
{
    // Jitter starting point within the first sample distance
    float2 uv = (uv0 + deltaUV) + randstep * deltaUV;
    
    // Snap first sample uv and initialize horizon tangent
    float2 snapped_duv = snap_uv_offset(uv - uv0);
    float3 T = tangent_vector(snapped_duv, dPdu, dPdv);
    float tanH = tangent(T) + g_TanAngleBias;

    float ao = 0;
    float h0 = 0;
    for(float j = 0; j < numSteps; ++j) {
        float2 snapped_uv = snap_uv_coord(uv);
        float3 S = fetch_eye_pos(snapped_uv);
        uv += deltaUV;

        // Ignore any samples outside the radius of influence
        float d2 = length2(S - P);
        if (d2 < g_sqr_R) {
            float tanS = tangent(P, S);

            [branch]
            if (tanS > tanH) {
                // Compute tangent vector associated with snapped_uv
                float2 snapped_duv = snapped_uv - uv0;
                float3 T = tangent_vector(snapped_duv, dPdu, dPdv);
                float tanT = tangent(T) + g_TanAngleBias;

                // Compute AO between tangent T and sample S
                float sinS = tan_to_sin(tanS);
                float sinT = tan_to_sin(tanT);
                float r = sqrt(d2) * g_inv_R;
                float h = sinS - sinT;
                ao += falloff(r) * (h - h0);
                h0 = h;

                // Update the current horizon angle
                tanH = tanS;
            }
        }
    }
    return ao;
}

//----------------------------------------------------------------------------------
float4 HORIZON_BASED_AO_PS( uniform bool useNormal, uniform int qualityMode, PostProc_VSOut IN ) : SV_TARGET
{
    float3 P = fetch_eye_pos(IN.texUV);
    
    // Project the radius of influence g_R from eye space to texture space.
    // The scaling by 0.5 is to go from [-1,1] to [0,1].
    float2 step_size = 0.5 * g_R  * g_FocalLen / P.z;

    // Early out if the projected radius is smaller than 1 pixel.
    float numSteps = min ( g_NumSteps, min(step_size.x * g_Resolution.x, step_size.y * g_Resolution.y));
    if( numSteps < 1.0 ) return 1.0;
    step_size = step_size / ( numSteps + 1 );

    // Nearest neighbor pixels on the tangent plane
    float3 Pr, Pl, Pt, Pb;
    float4 tangentPlane;
    if (useNormal) {
        float3 N = normalize(tNormal.SampleLevel(samNearest, float3(IN.texUV, 0), 0).xyz);
        tangentPlane = float4(N, dot(P, N));
        Pr = tangent_eye_pos(IN.texUV + float2(g_InvResolution.x, 0), tangentPlane);
        Pl = tangent_eye_pos(IN.texUV + float2(-g_InvResolution.x, 0), tangentPlane);
        Pt = tangent_eye_pos(IN.texUV + float2(0, g_InvResolution.y), tangentPlane);
        Pb = tangent_eye_pos(IN.texUV + float2(0, -g_InvResolution.y), tangentPlane);
    } else {
        Pr = fetch_eye_pos(IN.texUV + float2(g_InvResolution.x, 0));
        Pl = fetch_eye_pos(IN.texUV + float2(-g_InvResolution.x, 0));
        Pt = fetch_eye_pos(IN.texUV + float2(0, g_InvResolution.y));
        Pb = fetch_eye_pos(IN.texUV + float2(0, -g_InvResolution.y));
        float3 N = normalize(cross(Pr - Pl, Pt - Pb));
        tangentPlane = float4(N, dot(P, N));
    }
    
    // Screen-aligned basis for the tangent plane
    float3 dPdu = min_diff(P, Pr, Pl);
    float3 dPdv = min_diff(P, Pt, Pb) * (g_Resolution.y * g_InvResolution.x);

    // (cos(alpha),sin(alpha),jitter)
    float3 rand_Dir = tRandom.Load(int3((int)IN.pos.x&63, (int)IN.pos.y&63, 0)).xyz;

    float ao = 0;
    float d;

    // this switch gets unrolled by the HLSL compiler
    switch (qualityMode)
    {
    case 0:
        for (d = 0; d < g_NumDir*0.5; ++d) {
            float2 deltaUV = float2(g_Dirs[d].x*rand_Dir.x - g_Dirs[d].y*rand_Dir.y, 
                                g_Dirs[d].x*rand_Dir.y + g_Dirs[d].y*rand_Dir.x) 
                                * step_size.xy;
            ao += AccumulatedHorizonOcclusion_LowQuality(deltaUV, IN.texUV, P, numSteps, rand_Dir.z);
        }
        ao *= 2.0;
        break;
    case 1:
        for (d = 0; d < g_NumDir; ++d) {
            float2 deltaUV = float2(g_Dirs[d].x*rand_Dir.x - g_Dirs[d].y*rand_Dir.y, 
                                g_Dirs[d].x*rand_Dir.y + g_Dirs[d].y*rand_Dir.x) 
                                * step_size.xy;
             ao += AccumulatedHorizonOcclusion(deltaUV, IN.texUV, P, numSteps, rand_Dir.z, dPdu, dPdv);
        }
        break;
    case 2:
        for (d = 0; d < g_NumDir; ++d) {
            float2 deltaUV = float2(g_Dirs[d].x*rand_Dir.x - g_Dirs[d].y*rand_Dir.y, 
                                g_Dirs[d].x*rand_Dir.y + g_Dirs[d].y*rand_Dir.x) 
                                * step_size.xy;
             ao += AccumulatedHorizonOcclusion_Quality(deltaUV, IN.texUV, P, numSteps, rand_Dir.z, dPdu, dPdv);
        }
        break;
    }

    return 1.0 - ao / g_NumDir * g_Contrast;
}

//----------------------------------------------------------------------------------
technique10 HORIZON_BASED_AO_NLD_LOWQUALITY_Pass
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, FullScreenQuadVS(false) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HORIZON_BASED_AO_PS(true, 0) ) );
        SetBlendState( DisableBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//----------------------------------------------------------------------------------
technique10 HORIZON_BASED_AO_LD_LOWQUALITY_Pass
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, FullScreenQuadVS(false) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HORIZON_BASED_AO_PS(false, 0) ) );
        SetBlendState( DisableBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//----------------------------------------------------------------------------------
technique10 HORIZON_BASED_AO_NLD_Pass
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, FullScreenQuadVS(false) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HORIZON_BASED_AO_PS(true, 1) ) );
        SetBlendState( DisableBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//----------------------------------------------------------------------------------
technique10 HORIZON_BASED_AO_LD_Pass
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, FullScreenQuadVS(false) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HORIZON_BASED_AO_PS(false, 1) ) );
        SetBlendState( DisableBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//----------------------------------------------------------------------------------
technique10 HORIZON_BASED_AO_NLD_QUALITY_Pass
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, FullScreenQuadVS(false) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HORIZON_BASED_AO_PS(true, 2) ) );
        SetBlendState( DisableBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
    }
}

//----------------------------------------------------------------------------------
technique10 HORIZON_BASED_AO_LD_QUALITY_Pass
{
    pass p0
    {
        SetVertexShader( CompileShader( vs_4_0, FullScreenQuadVS(false) ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, HORIZON_BASED_AO_PS(false, 2) ) );
        SetBlendState( DisableBlend, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
        SetDepthStencilState( DisableDepth, 0 );
    }
}
