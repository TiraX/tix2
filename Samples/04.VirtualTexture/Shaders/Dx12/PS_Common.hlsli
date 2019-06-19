struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoord0 : TexCoord0;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 view : TexCoord1;
};

cbuffer EB_Primitive : register(b2)
{
	float4x4 WorldTransform;
	float4 VTUVTransform;
	float4 VTDebugInfo;
};