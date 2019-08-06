#include "Common.hlsli"
#include "VS_Instanced.hlsli"
#include "S_GrassRS.hlsli"

[RootSignature(Grass_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

	float3 WorldPosition = GetWorldPosition(vsInput);
	vsOutput.position = mul(float4(WorldPosition, 1.0), ViewProjection);
	vsOutput.texcoord0 = GetTextureCoords(vsInput) * 0.98 + 0.01;	// grass model uv shrink a little bit, or else will have border artifact. (Also in UE4)

    vsOutput.normal = vsInput.normal * 2.0 - 1.0;
    vsOutput.tangent = vsInput.tangent * 2.0 - 1.0;
	vsOutput.view = ViewPos - vsInput.position;

    return vsOutput;
}
