#include "S_GrassRS.hlsli"

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

SamplerState sampler0 : register(s0);

[RootSignature(Grass_RootSig)]
float4 main(VSOutput input, out float4 uvLayer : SV_Target1) : SV_Target0
{
	float4 BaseColor = texBaseColor.Sample(sampler0, input.uv);
	clip(BaseColor.w - 0.1);

	float4 Color = float4(0, 0, 0, 1);
	Color.xyz = BaseColor.xyz;

	// output uv
	uvLayer = float4(input.uv.xy, 0, 1);
	
	return Color;
}
