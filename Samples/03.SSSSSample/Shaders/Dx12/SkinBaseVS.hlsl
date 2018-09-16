#include "SkinBaseRS.hlsli"

cbuffer VSConstants : register(b0)
{
    float4x4 vp;
};

struct VSInput
{
    float3 position : POSITION;
	float3 normal : NORMAL;
    float2 texcoord0 : TEXCOORD;
    float3 tangent : TANGENT;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
    float3 bitangent : Bitangent;
};

[RootSignature(SkinBase_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

    vsOutput.position = mul(float4(vsInput.position, 1.0), vp);
    vsOutput.texCoord = vsInput.texcoord0;

    vsOutput.normal = vsInput.normal;
    vsOutput.tangent = vsInput.tangent;
	vsOutput.bitangent = cross(vsInput.normal, vsInput.tangent);;

    return vsOutput;
}
