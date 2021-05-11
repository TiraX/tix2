struct VSOutput
{
	float4 position : SV_Position;
}; 

cbuffer EB_View : register(b0)
{
	float4x4 ViewProjection;
	float3 ViewDir;
	float3 ViewPos;
	float3 MainLightDirection;
	float3 MainLightColor;
};