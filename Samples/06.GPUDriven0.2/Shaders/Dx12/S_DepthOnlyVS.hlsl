#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "VS_Instanced.hlsli"

struct DepthOnlyVSInput
{
	float3 position : POSITION;
	float4 ins_transition : INS_TRANSITION;
	float4 ins_transform0 : INS_TRANSFORM0;
	float4 ins_transform1 : INS_TRANSFORM1;
	float4 ins_transform2 : INS_TRANSFORM2;
};

struct DepthOnlyVSOutput
{
	float4 position : SV_Position;
};

float3 GetDepthOnlyWorldPosition(in DepthOnlyVSInput vsInput)
{
	float3x3 RotMat = float3x3(vsInput.ins_transform0.xyz, vsInput.ins_transform1.xyz, vsInput.ins_transform2.xyz);
	float3 position = mul(vsInput.position, RotMat);
	//float3 position = vsInput.position;
	position += vsInput.ins_transition.xyz;

	return position;
}

[RootSignature(BasePass_RootSig)]
DepthOnlyVSOutput main(DepthOnlyVSInput vsInput)
{
	DepthOnlyVSOutput vsOutput;

	float3 WorldPosition = GetDepthOnlyWorldPosition(vsInput);
	vsOutput.position = mul(float4(WorldPosition, 1.0), ViewProjection);
    return vsOutput;
}
