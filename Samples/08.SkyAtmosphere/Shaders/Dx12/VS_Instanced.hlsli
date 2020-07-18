
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
	float3 normal : NORMAL;
    float2 texcoord0 : TEXCOORD;
    float3 tangent : TANGENT;
	float4 ins_transition : INS_TRANSITION;
	float4 ins_transform0 : INS_TRANSFORM0;
	float4 ins_transform1 : INS_TRANSFORM1;
	float4 ins_transform2 : INS_TRANSFORM2;
};

struct VSOutput
{
    float4 position : SV_Position;
    float2 texcoord0 : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
	float3 view : TexCoord1;
};

float3x3 GetWorldRotationMat(in VSInput vsInput)
{
	return float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
}

float3 GetWorldPosition(in VSInput vsInput)
{
	float3x3 RotMat = float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
	float3 position = mul(vsInput.position, RotMat);
	//float3 position = vsInput.position;
	position += vsInput.ins_transition.xyz;

	return position;
}

half3 TransformNormal(in half3 Normal, in half3x3 RotMat)
{
	return mul(Normal, RotMat);
}

float2 GetTextureCoords(in VSInput vsInput)
{
	return vsInput.texcoord0;
}

//float4 GetVTTextureCoords(in VSInput vsInput, in float4 VTUVTransform)
//{
//	float4 TexCoord;
//	TexCoord.xy = (vsInput.texcoord0) * VTUVTransform.zw + VTUVTransform.xy;
//	TexCoord.zw = VTDebugInfo.xy;
//	TexCoord.z = vsInput.texcoord0.y;
//	return TexCoord;
//}