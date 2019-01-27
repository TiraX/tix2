#include "S_SkyDome.hlsli"

cbuffer EB_View : register(b0)
{
	float4x4 ViewProjection;
	float3 ViewDir;
	float3 ViewPos;
};

struct VSInput
{
    float3 position : POSITION;
	float3 normal : NORMAL;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 normal : Normal;
};

[RootSignature(SkyDome_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;
	
	// hack , manually scale
    vsOutput.position = mul(float4(vsInput.position, 1.0), ViewProjection);

    vsOutput.normal = vsInput.normal * 2.0 - 1.0;

    return vsOutput;
}
