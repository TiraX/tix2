#include "S_GrassRS.hlsli"
#include "PS_Common.hlsli"
#include "PS_VT.hlsli"

Texture2D<float4> texBaseColor : register(t0);
SamplerState sampler0 : register(s0);

[RootSignature(Grass_RootSig)]
float4 main(VSOutput input, out float4 uvLayer : SV_Target1) : SV_Target0
{
	float4 BaseColor = texBaseColor.Sample(sampler0, input.uv.xy);
	clip(BaseColor.w - 0.1);

	float4 Color = float4(0, 0, 0, 1);
	Color.xyz = BaseColor.xyz;

	// output uv
	uvLayer = float4(input.uv.xy, mip_map_level(input.uv.zw, 1024), 1);
	uvLayer = input.uv;
	
	return Color;
}
