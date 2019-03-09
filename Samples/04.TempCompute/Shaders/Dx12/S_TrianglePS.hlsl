#include "S_TriangleRS.hlsli"

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : TEXCOORD;
};

// A pass-through function for the (interpolated) color data.
[RootSignature(Triangle_RootSig)]
float4 main(PixelShaderInput input) : SV_TARGET
{
	return input.color;
}
