#include "SkinBaseRS.hlsli"

struct VSOutput
{
    sample float4 position : SV_Position;
    sample float2 uv : TexCoord0;
    sample float3 normal : Normal;
    sample float3 tangent : Tangent;
    sample float3 bitangent : Bitangent;
};

Texture2D<float3> texDiffuse		: register(t0);
Texture2D<float3> texNormal			: register(t1);
Texture2D<float3> texSpecular		: register(t2);


cbuffer PSConstants : register(b0)
{
    float3 SunDirection;
    float3 SunColor;
    float3 AmbientColor;
    float4 ShadowTexelSize;

    float4 InvTileDim;
    uint4 TileCount;
    uint4 FirstLightIndex;
}

SamplerState sampler0 : register(s0);

[RootSignature(SkinBase_RootSig)]
float4 main(VSOutput vsOutput) : SV_Target0
{
	float3 n = texNormal.Sample(sampler0, vsOutput.uv);
	n = n * 2.0 - 1.0;
	n.z = sqrt(1.0 - n.x * n.x - n.y * n.y);
	//texDiffuse.Sample(sampler0, vsOutput.uv);

	n = n * 0.5 + 0.5;

    return float4(n, 1.0);
}
