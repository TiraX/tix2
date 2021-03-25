#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "PS_Common.hlsli"

struct VSOutputWithColor
{
	float4 position : SV_Position;
	float2 texcoord0 : TexCoord0;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 view : TexCoord1;
	float4 color : TexCoord2;
};

[RootSignature(BasePass_RootSig)]
float4 main(VSOutputWithColor input) : SV_Target0
{
	float Light = saturate(dot(MainLightDirection, input.normal));
	//return float4(Light, Light, Light,1);
	return input.color;
}
