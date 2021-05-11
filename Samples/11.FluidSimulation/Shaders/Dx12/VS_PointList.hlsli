
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

struct VSInput
{
    float3 position : POSITION;
};

struct VSOutput
{
    float4 position : SV_Position;
};

float3x3 GetWorldRotationMat(in VSInput vsInput)
{
	return float3x3(float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1));
}

float3 GetWorldPosition(in VSInput vsInput)
{
	return vsInput.position;
}

half3 TransformNormal(in half3 Normal, in half3x3 RotMat)
{
	return half3(0, 0, 1);
}

float2 GetTextureCoords(in VSInput vsInput)
{
	return float2(0, 0);
}

//float4 GetVTTextureCoords(in VSInput vsInput, in float4 VTUVTransform)
//{
//	float4 TexCoord;
//	TexCoord.xy = (vsInput.texcoord0) * VTUVTransform.zw + VTUVTransform.xy;
//	TexCoord.zw = VTDebugInfo.xy;
//	TexCoord.z = vsInput.texcoord0.y;
//	return TexCoord;
//}