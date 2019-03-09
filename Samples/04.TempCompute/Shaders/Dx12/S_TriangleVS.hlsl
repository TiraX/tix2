#include "S_TriangleRS.hlsli"

cbuffer SceneConstantBuffer : register(b0)
{
	float4 velocity;
	float4 offset;
	float4 color;
	float4x4 projection;
};
// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float4 color : TEXCOORD;
};

[RootSignature(Triangle_RootSig)]
PixelShaderInput main(VertexShaderInput vsInput)
{
	PixelShaderInput output;
	float4 pos = float4(vsInput.pos, 1.0f);

	// vertex input position already in projected space.
	output.pos = pos;

	output.color = color;

	return output;
}
