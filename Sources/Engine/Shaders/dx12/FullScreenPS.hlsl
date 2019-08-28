// Per-pixel color data passed through the pixel shader.
#include "FullScreenRS.hlsli"

struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

Texture2D MI_Texture : register(t0);
SamplerState g_sampler : register(s0);

// A pass-through function for the (interpolated) color data.
[RootSignature(FullScreen_RootSig)]
float4 main(PixelShaderInput input) : SV_TARGET
{
	//return float4(input.uv, 0.0, 1.0);
	float4 color = MI_Texture.Sample(g_sampler, input.uv);
	
	// Do gamma
	color.rgb = sqrt(color.rgb);
	return color;
}
