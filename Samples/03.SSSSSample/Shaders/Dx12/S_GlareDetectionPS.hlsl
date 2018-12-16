//#include "S_SSSBlur.hlsli"

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer GlareDetectionParam : register(b0)
{
	float4 GlareDetectionParam;	// x=exposure, y=threshold,
};

Texture2D TexBaseColor : register(t0);

SamplerState sampler0 : register(s0);

// A pass-through function for the (interpolated) color data.
//[RootSignature(SSSBlur_RootSig)]
float4 main(PixelShaderInput input) : SV_TARGET
{
	float exposure = GlareDetectionParam.x;
	float bloomThreshold = GlareDetectionParam.y;

	float4 color = TexBaseColor.Sample(sampler0, input.uv);
	color.rgb *= exposure;

	return float4(max(color.rgb - bloomThreshold / (1.0 - bloomThreshold), 0.0), color.a);
}
