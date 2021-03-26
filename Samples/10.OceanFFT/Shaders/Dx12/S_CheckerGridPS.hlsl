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
	float4 world_position : TexCoord2;
	float4 color : TexCoord3;
};

[RootSignature(BasePass_RootSig)]
float4 main(VSOutputWithColor input) : SV_Target0
{
	// calc normal
	float3 nx = normalize(ddx(input.world_position.xyz));
	float3 ny = normalize(ddy(input.world_position.xyz));
	float3 n = cross(nx, ny);
	float Light = saturate(dot(MainLightDirection, n));
	//return float4(Light, Light, Light,1);
	return input.color;// *float4(n * 0.5f + 0.5f, 1.f);
}
