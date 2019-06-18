
cbuffer EB_View : register(b0)
{
	float4x4 ViewProjection;
	float3 ViewDir;
	float3 ViewPos;
};

cbuffer EB_Primitive : register(b1)
{
	float4x4 WorldTransform;
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
	half4 ins_transform0 : INS_TRANSFORM0;
	half4 ins_transform1 : INS_TRANSFORM1;
	half4 ins_transform2 : INS_TRANSFORM2;
};

struct VSOutput
{
    float4 position : SV_Position;
    float4 texCoord : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
	float3 view : TexCoord1;
};

float3 GetWorldPosition(in VSInput vsInput)
{
	float3x3 RotMat = float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
	float3 position = mul(vsInput.position, RotMat);
	position += vsInput.ins_transition.xyz;

	return position;
}

float4 GetVTTextureCoords(in VSInput vsInput, in float4 VTUVTransform)
{
	float4 TexCoord;
	// temp , use frac to keep uv in 0.0-1.0.
	// this is not correct, should use frac in pixel shader, not vertex
	TexCoord.xy = frac(vsInput.texcoord0) * VTUVTransform.zw + VTUVTransform.xy;
	TexCoord.zw = VTDebugInfo.xy;
	TexCoord.z = (vsInput.texcoord0.y);
	//TexCoord.zw = vsInput.texcoord0;
	return TexCoord;
}