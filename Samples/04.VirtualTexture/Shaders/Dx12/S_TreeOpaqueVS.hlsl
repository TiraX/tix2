#include "S_BasePassRS.hlsli"
#include "VS_Instanced.hlsli"

[RootSignature(BasePass_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

	float3 WorldPosition = GetWorldPosition(vsInput);
	vsOutput.position = mul(float4(WorldPosition, 1.0), ViewProjection);
	vsOutput.texCoord = GetVTTextureCoords(vsInput, VTUVTransform);

    vsOutput.normal = vsInput.normal * 2.0 - 1.0;
    vsOutput.tangent = vsInput.tangent * 2.0 - 1.0;
	vsOutput.view = ViewPos - vsInput.position;

    return vsOutput;
}
