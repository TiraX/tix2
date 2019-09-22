#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "PS_Common.hlsli"

[RootSignature(BasePass_RootSig)]
float4 main(VSOutput input) : SV_Target0
{
	return float4(1,0,1,1);
}
