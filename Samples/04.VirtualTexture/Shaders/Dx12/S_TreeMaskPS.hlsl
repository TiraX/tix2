#include "S_BasePassRS.hlsli"
#include "PS_Common.hlsli"
#include "PS_VT.hlsli"

Texture2D<float4> texBaseColor : register(t0);
Texture2D<float3> texNormal : register(t1);
SamplerState sampler0 : register(s0);

[RootSignature(BasePass_RootSig)]
float4 main(VSOutput input, out float4 uvLayer : SV_Target1) : SV_Target0
{
	float4 BaseColor = texBaseColor.Sample(sampler0, input.uv.xy);
	clip(BaseColor.w - 0.2);
	
	float3 Normal = texNormal.Sample(sampler0, input.uv.xy).xyz;

	float4 Color = float4(0, 0, 0, 1);
	Color.xyz = BaseColor.xyz * Normal.z;

	// output uv
	uvLayer = float4(input.uv.xy, mip_map_level(input.uv.zw), 1);
	
	return Color;
}
