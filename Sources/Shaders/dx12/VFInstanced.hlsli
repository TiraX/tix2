
// Vertex factory input structure
struct VFInput
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

// Interporlate to PS
struct VStoPS
{
    float4 position : SV_Position;
    float2 texcoord0 : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
	float3 view : TexCoord1;
};