#include "S_SkyAtmosphereRS.hlsli"

// Simple shader to do vertex processing on the GPU.
[RootSignature(Sky_RootSig)]
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos.xy, 0.9999999f, 1.0f);

	// vertex input position already in projected space.
	output.pos = pos;

	output.uv = input.uv;

	return output;
}