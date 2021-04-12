#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "PS_Common.hlsli"

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

#define CheckerGrid_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
	"CBV(b0, visibility=SHADER_VISIBILITY_VERTEX), " \
	"CBV(b1, visibility=SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(SRV(t0, numDescriptors=1), visibility=SHADER_VISIBILITY_VERTEX)," \
    "DescriptorTable(SRV(t1, numDescriptors=1), visibility=SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_WRAP, " \
                      "addressV = TEXTURE_ADDRESS_WRAP, " \
                      "addressW = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR, "\
                    "visibility = SHADER_VISIBILITY_VERTEX)," \
    "StaticSampler(s1, addressU = TEXTURE_ADDRESS_WRAP, " \
                      "addressV = TEXTURE_ADDRESS_WRAP, " \
                      "addressW = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR, "\
                    "visibility = SHADER_VISIBILITY_PIXEL)"

Texture2D NormalTex : register(t1);
SamplerState sampler1 : register(s1);

[RootSignature(CheckerGrid_RootSig)]
float4 main(VSOutputWithColor input) : SV_Target0
{
	//return NormalTex.Sample(sampler1, input.texcoord0);

	// calc normal
	float3 nx = normalize(ddx(input.world_position.xyz));
	float3 ny = normalize(ddy(input.world_position.xyz));
	float3 n = cross(nx, ny);
	float Light = saturate(dot(MainLightDirection, n));
	//return float4(Light, Light, Light,1);
	//return float4(n * 0.5f + 0.5f, 1.f);
	return input.color * float4(n * 0.5f + 0.5f, 1.f);
}
