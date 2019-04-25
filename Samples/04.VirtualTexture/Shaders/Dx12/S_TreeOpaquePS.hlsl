#include "S_BasePassRS.hlsli"

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
	float3 view : TexCoord1;
	float4 worldPosition : TexCoord2;
};

Texture2D<float4> texBaseColor : register(t0);
Texture2D<float3> texNormal : register(t1);

#define MAX_LIGHTS 32

cbuffer EB_Primitive : register(b4)
{
	float4x4 WorldTransform;
};

cbuffer EB_Lights : register(b5)
{
	float4 LightPosition[MAX_LIGHTS];
	float4 LightColor[MAX_LIGHTS];
};

SamplerState sampler0 : register(s0);

[RootSignature(BasePass_RootSig)]
float4 main(VSOutput input) : SV_Target0
{
	float3 BaseColor = texBaseColor.Sample(sampler0, input.uv).xyz;
	float3 Normal = texNormal.Sample(sampler0, input.uv).xyz;

	float4 Color = float4(0, 0, 0, 1);
	Color.xyz = BaseColor * Normal.z;
	
	return Color;
}
