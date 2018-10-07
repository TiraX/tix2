#include "SkinBaseRS.hlsli"

struct VSOutput
{
    sample float4 position : SV_Position;
    sample float2 uv : TexCoord0;
    sample float3 normal : Normal;
    sample float3 tangent : Tangent;
    sample float3 bitangent : Bitangent;
};

Texture2D<float3> texDiffuse : register(t0);
Texture2D<float3> texNormal : register(t1);
Texture2D<float3> texSpecular : register(t2);
Texture2D<float3> texBeckmann : register(t3);
TextureCube<float4> texIrrMap : register(t4);

#define N_LIGHTS 3


cbuffer LightBinding : register(b14)
{
	int4 LightCount;
	int4 LightIndex;
};

struct PointLight
{
	float3 Position;
	float3 Color;
};
ConstantBuffer<PointLight> LightsBuffer[64] : register(b15);

SamplerState sampler0 : register(s0);

[RootSignature(SkinBase_RootSig)]
float4 main(VSOutput vsOutput) : SV_Target0
{
	//float3 n = texBeckmann.Sample(sampler0, vsOutput.uv);

	// HACK for using baked object space normals (because blender tangent export is not working)
	//float3 n = texBeckmann.Sample(sampler0, vsOutput.uv);
	float3 objSpaceNormal = normalize(texNormal.Sample(sampler0, vsOutput.uv).xyz * 2.0 - 1.0);

	float3 worldSpaceNormal = objSpaceNormal;
	float3 normal = worldSpaceNormal;

	//input.view = normalize(input.view);

	// Fetch albedo, specular parameters and static ambient occlusion:
	float3 albedo = texDiffuse.Sample(sampler0, vsOutput.uv);
	float3 specularAO = texSpecular.Sample(sampler0, vsOutput.uv).rgb;

	float occlusion = specularAO.r;
	float intensity = specularAO.b * 1.0;// specularIntensity;
	float roughness = (specularAO.g / 0.3) * 1.0;// specularRoughness;

	// Initialize the output:
	float4 color = float4(0.0, 0.0, 0.0, 0.0);
	float4 specularColor = float4(0.0, 0.0, 0.0, 0.0);

	//*
	[unroll]
	for (int i = 0; i < LightCount.x; i++)
	{
		int Index = LightIndex[i];
		color.xyz += LightsBuffer[Index].Color;
//		float3 light = lights[i].position - input.worldPosition;
//		float dist = length(light);
//		light /= dist;
//
//		float spot = dot(lights[i].direction, -light);
//		[flatten]
//		if (spot > lights[i].falloffStart) {
//			// Calculate attenuation:
//			float curve = min(pow(dist / lights[i].farPlane, 6.0), 1.0);
//			float attenuation = lerp(1.0 / (1.0 + lights[i].attenuation * dist * dist), 0.0, curve);
//
//			// And the spot light falloff:
//			spot = saturate((spot - lights[i].falloffStart) / lights[i].falloffWidth);
//
//			// Calculate some terms we will use later on:
//			float3 f1 = lights[i].color * attenuation * spot;
//			float3 f2 = albedo.rgb * f1;
//
//			// Calculate the diffuse and specular lighting:
//			float3 diffuse = saturate(dot(light, normal));
//			float specular = intensity * SpecularKSK(beckmannTex, normal, light, input.view, roughness);
//
//			// And also the shadowing:
//			float shadow = ShadowPCF(input.worldPosition, i, 3, 1.0);
//
//			// Add the diffuse and specular components:
//#ifdef SEPARATE_SPECULARS
//			color.rgb += shadow * f2 * diffuse;
//			specularColor.rgb += shadow * f1 * specular;
//#else
//			color.rgb += shadow * (f2 * diffuse + f1 * specular);
//#endif
//
//			// Add the transmittance component:
//			if (sssEnabled && translucencyEnabled)
//				color.rgb += f2 * albedo.a * SSSSTransmittance(translucency, sssWidth, input.worldPosition, input.normal, light, shadowMaps[i], lights[i].viewProjection, lights[i].farPlane);
//		}
	}
	//*/

	// Add the ambient component:
	//color.rgb += occlusion * ambient * albedo.rgb * irradianceTex.Sample(LinearSampler, normal).rgb;
	color.rgb += occlusion * albedo.rgb ;

	// Store the SSS strength:
	//specularColor.a = albedo.a;

	// Store the depth value:
	//depth = input.svPosition.w;

	// Convert to non-homogeneous points by dividing by w:
	//input.currPosition.xy /= input.currPosition.z; // w is stored in z
	//input.prevPosition.xy /= input.prevPosition.z;

	// Calculate velocity in non-homogeneous projection space:
	//velocity = input.currPosition.xy - input.prevPosition.xy;

	// Compress the velocity for storing it in a 8-bit render target:
	//color.a = sqrt(5.0 * length(velocity));
	float3 env = texIrrMap.Sample(sampler0, vsOutput.normal).xyz;
	float3 diff = texDiffuse.Sample(sampler0, vsOutput.uv);

	return float4(env, 1.0);
	//return color;
	//return float4(specularAO.rrr, 1.0);

    //return float4(n, 1.0);
}
