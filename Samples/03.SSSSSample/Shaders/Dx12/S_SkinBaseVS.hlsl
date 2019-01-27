#include "S_SkinBaseRS.hlsli"

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
    float2 texcoord0 : TEXCOORD;
    float3 tangent : TANGENT;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texCoord : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
	float3 view : TexCoord1;
	float4 worldPosition : TexCoord2;
};

[RootSignature(SkinBase_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

    vsOutput.position = mul(float4(vsInput.position, 1.0), ViewProjection);
    vsOutput.texCoord = vsInput.texcoord0;
	vsOutput.texCoord.y = 1.0 - vsOutput.texCoord.y;

    vsOutput.normal = vsInput.normal * 2.0 - 1.0;
    vsOutput.tangent = vsInput.tangent * 2.0 - 1.0;
	vsOutput.view = ViewPos - vsInput.position;
	vsOutput.worldPosition.xyz = vsInput.position;
	vsOutput.worldPosition.w = vsOutput.position.z / vsOutput.position.w;

    return vsOutput;
}
