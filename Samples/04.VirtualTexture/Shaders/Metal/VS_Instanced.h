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
    
    float4 ins_transition [[attribute(4)]];
    float4 ins_transform0 [[attribute(5)]];
    float4 ins_transform1 [[attribute(6)]];
    float4 ins_transform2 [[attribute(7)]];
    
} VSInput;

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

float3 GetWorldPosition(VSInput vsInput)
{
    float3x3 RotMat = float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
    float3 position = RotMat * vsInput.position;
    position += vsInput.ins_transition.xyz;

    return position;
}

half2 GetTextureCoords(VSInput vsInput)
{
	return vsInput.texcoord0;
}
