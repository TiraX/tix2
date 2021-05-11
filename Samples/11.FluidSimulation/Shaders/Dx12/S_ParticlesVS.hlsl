#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "VS_PointList.hlsli"

[RootSignature(BasePass_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

	float3 WorldPosition = GetWorldPosition(vsInput);
	vsOutput.position = mul(float4(WorldPosition, 1.0), ViewProjection);

    return vsOutput;
}
