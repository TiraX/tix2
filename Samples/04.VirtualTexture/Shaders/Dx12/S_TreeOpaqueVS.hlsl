#include "S_BasePassRS.hlsli"

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
    float2 texCoord : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
	float3 view : TexCoord1;
	float4 worldPosition : TexCoord2;
};

[RootSignature(BasePass_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

	float3x3 RotMat = float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
	float3 position = mul(vsInput.position, RotMat);
	position += vsInput.ins_transition.xyz;
    vsOutput.position = mul(float4(position, 1.0), ViewProjection);
	vsOutput.texCoord = vsInput.texcoord0 * VTUVTransform.zw + VTUVTransform.xy;
	//vsOutput.texCoord.y = 1.0 - vsOutput.texCoord.y;

    vsOutput.normal = vsInput.normal * 2.0 - 1.0;
    vsOutput.tangent = vsInput.tangent * 2.0 - 1.0;
	vsOutput.view = ViewPos - vsInput.position;
	vsOutput.worldPosition.xyz = vsInput.position;
	vsOutput.worldPosition.w = vsOutput.position.z / vsOutput.position.w;

    return vsOutput;
}
