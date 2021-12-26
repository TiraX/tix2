#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "VS_Instanced.hlsli"

[RootSignature(BasePass_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

	float3 WorldPosition = GetWorldPosition(vsInput);
	vsOutput.position = mul(float4(WorldPosition, 1.0), ViewProjection);
	vsOutput.texcoord0 = GetTextureCoords(vsInput);

	half3x3 RotMat = GetWorldRotationMat(vsInput);
    vsOutput.normal = TransformNormal(vsInput.normal * 2.0 - 1.0, RotMat);
    vsOutput.tangent = TransformNormal(vsInput.tangent * 2.0 - 1.0, RotMat);
	vsOutput.view = ViewPos - vsInput.position;
	vsOutput.worldPosition = WorldPosition;

    return vsOutput;
}
