#include "S_Combine.hlsli"

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer BloomParam : register(b0)
{
	float4 BloomParam;	// x = exposure, y = bloom intensity, 
};

Texture2D TexSource : register(t0);
Texture2D TexBloom0 : register(t1);
Texture2D TexBloom1 : register(t2);

SamplerState sampler0 : register(s0);

float3 FilmicTonemap(float3 x) {
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	float W = 11.2;
	return ((x*(A*x + C * B) + D * E) / (x*(A*x + B) + D * F)) - E / F;
}

float3 DoToneMap(float3 color) 
{
	float exposure = BloomParam.x;
	color = 2.0f * FilmicTonemap(exposure * color);
	float3 whiteScale = 1.0f / FilmicTonemap(11.2);
	color *= whiteScale;
	return color;
}

// A pass-through function for the (interpolated) color data.
[RootSignature(Combine_RootSig)]
float4 main(PixelShaderInput input) : SV_TARGET
{
	const float w[] = { 2.0 / 3.0, 1.0 / 3.0 };
	float bloomIntensity = BloomParam.y;

	//float4 color = PyramidFilter(finalTex, texcoord, pixelSize * defocus);
	float4 color = TexSource.Sample(sampler0, input.uv);

	float4 b0 = TexBloom0.Sample(sampler0, input.uv);
	float4 b1 = TexBloom1.Sample(sampler0, input.uv);

	color.xyz += bloomIntensity * w[0] * b0.xyz;
	color.xyz += bloomIntensity * w[1] * b1.xyz;

	color.rgb = sqrt(DoToneMap(color.rgb));
	return color;
}
