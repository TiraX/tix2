#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "PS_Common.hlsli"

[RootSignature(BasePass_RootSig)]
float4 main(VSOutput input) : SV_Target0
{
	float Light = saturate(dot(MainLightDirection, input.normal));
	//return float4(Light, Light, Light,1);
	return float4(input.normal * 0.5 + 0.5, 1.0);
}
