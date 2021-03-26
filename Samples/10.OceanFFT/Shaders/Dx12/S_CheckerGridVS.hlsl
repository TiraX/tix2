#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "VS_Instanced.hlsli"
struct VSInputWithColor
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 texcoord0 : TEXCOORD;
	float3 tangent : TANGENT;
	float4 ins_transition : INS_TRANSITION;
	float4 ins_transform0 : INS_TRANSFORM0;
	float4 ins_transform1 : INS_TRANSFORM1;
	float4 ins_transform2 : INS_TRANSFORM2;
};

struct VSOutputWithColor
{
	float4 position : SV_Position;
	float2 texcoord0 : TexCoord0;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 view : TexCoord1;
	float4 world_position : TexCoord2;
	float4 color : TexCoord3;
};


float3 GetWorldPosition(in VSInputWithColor vsInput)
{
	float3x3 RotMat = float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
	float3 position = mul(vsInput.position, RotMat);
	//float3 position = vsInput.position;
	position += vsInput.ins_transition.xyz;

	return position;
}

float3x3 GetWorldRotationMat(in VSInputWithColor vsInput)
{
	return float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
}
#define CheckerGrid_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
	"CBV(b0, visibility=SHADER_VISIBILITY_VERTEX), " \
	"CBV(b1, visibility=SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_VERTEX)," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_WRAP, " \
                      "addressV = TEXTURE_ADDRESS_WRAP, " \
                      "addressW = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR, "\
                    "visibility = SHADER_VISIBILITY_VERTEX)"
	
Texture2D OceanDisplacementTex : register(t0);
SamplerState sampler0 : register(s0);

float remapto01(float value, float bot, float top)
{
	value = clamp(value , bot, top);
	value /= (top - bot);
	return value;
}

[RootSignature(CheckerGrid_RootSig)]
VSOutputWithColor main(VSInputWithColor vsInput)
{
	VSOutputWithColor vsOutput;

	float3 WorldPosition = GetWorldPosition(vsInput);

	// Ocean displacement
	float4 OceanDisplacement = OceanDisplacementTex.SampleLevel(sampler0, vsInput.texcoord0, 0);
	WorldPosition += OceanDisplacement.xyz;

	vsOutput.position = mul(float4(WorldPosition, 1.0), ViewProjection);
	vsOutput.texcoord0 = vsInput.texcoord0;

	half3x3 RotMat = GetWorldRotationMat(vsInput);
    vsOutput.normal = TransformNormal(vsInput.normal * 2.0 - 1.0, RotMat);
    vsOutput.tangent = TransformNormal(vsInput.tangent * 2.0 - 1.0, RotMat);
	vsOutput.view = ViewPos - vsInput.position;
	vsOutput.world_position = float4(WorldPosition, 1.f);
	float mask = OceanDisplacement.w;// +(5.5f);
	float c = saturate(-mask / 1.5 - 1.6);
	vsOutput.color = float4(c, 0, 0.1, 1.f);

    return vsOutput;
}
