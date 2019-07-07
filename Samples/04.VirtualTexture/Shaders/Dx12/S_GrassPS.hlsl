#include "Common.hlsli"
#include "PS_Common.hlsli"
#include "PS_VT.hlsli"
#include "S_GrassRS.hlsli"

[RootSignature(Grass_RootSig)]
float4 main(VSOutput input, out float4 uvLayer : SV_Target1) : SV_Target0
{
	float4 BaseColor = GetBaseColor(input.texcoord0.xy);
	clip(BaseColor.w - 0.1);

	// output uv
	//uvLayer = float4(input.texcoord0.xy, mip_map_level(input.texcoord0.zw, 1024), 1); 
	uvLayer = GetVTTextureCoords(input.texcoord0);
	
	return BaseColor;
}
