struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoord0 : TexCoord0;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 view : TexCoord1;
}; 

cbuffer EB_View : register(b2)
{
	float4x4 ViewProjection;
	float3 ViewDir;
	float3 ViewPos;
	float3 MainLightDirection;
	float3 MainLightColor;
};