//
//  VS_Instanced.h
//  VirtualTexture
//
//  Created by Tirax on 2019/8/11.
//  Copyright © 2019 zhaoshuai. All rights reserved.
//

typedef struct
{
    float3 position [[attribute(0)]];
    half3 normal [[attribute(1)]];
    half2 texcoord0 [[attribute(2)]];
    half3 tangent [[attribute(3)]];
    float4 ins_transition[[attribute(4)]];
    half4 ins_transform0[[attribute(5)]];
    half4 ins_transform1[[attribute(6)]];
    half4 ins_transform2[[attribute(7)]];
} VertexInput;

typedef struct
{
    float4 position [[position]];
    float2 texcoord0;
    half3 normal;
    half3 tangent;
	half3 view;
    float4 worldPosition;
} VSOutput;

typedef struct
{
    float4x4 ViewProjection;
    float3 ViewDir;
    float3 ViewPos;
} EB_View;

typedef struct
{
	float4x4 WorldTransform;
	float4 VTUVTransform;
	float4 VTDebugInfo;
} EB_Primitive;

inline float3 GetWorldPosition(VertexInput vsInput)
{
    half3x3 RotMat = half3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
    float3 position = float3(RotMat * half3(vsInput.position));
    position += vsInput.ins_transition.xyz;

    return position;
}

inline float2 GetTextureCoords(VertexInput vsInput, float4 VTTransform)
{
	//return (clamp(float2(vsInput.texcoord0), 0.f, 0.99f) * VTTransform.zw + VTTransform.xy);
    return float2(vsInput.texcoord0);

}
