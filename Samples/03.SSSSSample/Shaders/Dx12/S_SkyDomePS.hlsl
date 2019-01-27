#include "S_SkyDome.hlsli"

struct VSOutput
{
	float4 position : SV_Position;
	float3 normal : Normal;
};

TextureCube<float3> texSkyMap : register(t0);

#define MAX_LIGHTS 32

cbuffer EB_Primitive : register(b4)
{
	int4 LightCount;
	int4 LightIndex;
};

cbuffer EB_LightData : register(b5)
{
	float4 LightPosition[MAX_LIGHTS];
	float4 LightColor[MAX_LIGHTS];
};

SamplerState sampler0 : register(s0);

[RootSignature(SkyDome_RootSig)]
float4 main(VSOutput input) : SV_Target0
{
	float3 normal = -normalize(input.normal);

	float4 Color;
	Color.xyz = sqrt(texSkyMap.Sample(sampler0, normal).xyz);
	Color.a = 1.0;
	return Color;
}
