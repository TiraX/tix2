#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "PS_PointList.hlsli"

[RootSignature(BasePass_RootSig)]
float4 main(VSOutput input) : SV_Target0
{
	return float4(0.5, 0.5, 0.5, 1.f);
}
