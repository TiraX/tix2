
cbuffer EB_View : register(b0)
{
	float4x4 ViewProjection;
	float3 ViewDir;
	float3 ViewPos;
	float3 MainLightDirection;
	float3 MainLightColor;
};

cbuffer EB_Primitive : register(b1)
{
	float4x4 LocalToWorld;
	float4 VTUVTransform;
	float4 VTDebugInfo;
};