#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "PS_Common.hlsli"

[RootSignature(BasePass_RootSig)]
float4 main(VSOutput input, out float4 OutNormal : SV_Target1) : SV_Target0
{
	OutNormal = float4(normalize(input.normal) * 0.5 + 0.5, 1.0);
	//OutNormal = float4(normalize(input.normal), 1.0);
	return float4(input.worldPosition, 1.0);
}
