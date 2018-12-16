//#include "S_SSSBlur.hlsli"

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

//[RootSignature(SSSBlur_RootSig)]
PixelShaderInput main(VertexShaderInput vsInput)
{
	PixelShaderInput output;
	float4 pos = float4(vsInput.pos, 1.0f);

	// vertex input position already in projected space.
	output.pos = pos;

	output.uv = vsInput.uv;

	return output;
}
