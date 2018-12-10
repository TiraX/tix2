#include "S_SkinBaseRS.hlsli"

struct VSOutput
{
    float4 position : SV_Position;
    float2 uv : TexCoord0;
    float3 normal : Normal;
    float3 tangent : Tangent;
	float3 view : TexCoord1;
	float3 worldPosition : TexCoord2;
};

Texture2D<float3> texDiffuse : register(t0);
Texture2D<float3> texNormal : register(t1);
Texture2D<float3> texSpecular : register(t2);
Texture2D<float3> texBeckmann : register(t3);
TextureCube<float4> texIrrMap : register(t4);

#define MAX_LIGHTS 32

cbuffer LightBinding : register(b4)
{
	int4 LightCount;
	int4 LightIndex;
};

cbuffer LightData : register(b5)
{
	float4 LightPosition[MAX_LIGHTS];
	float4 LightColor[MAX_LIGHTS];
}

SamplerState sampler0 : register(s0);


static const float specularIntensity = 1.84;
static const float specularRoughness = 0.3;
static const float specularFresnel = 0.81;

float3 BumpMap(Texture2D<float3> normalTex, float2 texcoord) {
	float3 bump;
	bump.xy = -1.0 + 2.0 * normalTex.Sample(sampler0, texcoord).gr;
	bump.z = sqrt(1.0 - bump.x * bump.x - bump.y * bump.y);
	return normalize(bump);
}

float Fresnel(float3 h, float3 view, float f0) {
	float base = 1.0 - dot(view, h);
	float exponential = pow(base, 5.0);
	return exponential + f0 * (1.0 - exponential);
}

float SpecularKSK(Texture2D<float3> beckmannTex, float3 normal, float3 light, float3 view, float roughness) {
	float3 h = view + light;
	float3 halfn = normalize(h);

	float ndotl = max(dot(normal, light), 0.0);
	float ndoth = max(dot(normal, halfn), 0.0);

	float ph = pow(2.0 * beckmannTex.SampleLevel(sampler0, float2(ndoth, roughness), 0).r, 10.0);
	float f = lerp(0.25, Fresnel(halfn, view, 0.028), specularFresnel);
	float ksk = max(ph * f / dot(h, h), 0.0);

	return ndotl * ksk;
}

float3 GetWorldSpaceNormal(in float3 objSpaceNormal, in VSOutput input)
{
	float3 iNormal = normalize(input.normal);
	float3 iTangent = normalize(input.tangent);
	float3 iBitangent = cross(iNormal, iTangent);

	float3x3 tbn = float3x3(iTangent, iBitangent, iNormal);
	return mul(objSpaceNormal, tbn);
	//float3x3 tbn = transpose(float3x3(iTangent, iBitangent, iNormal));
	//return mul(tbn, objSpaceNormal);
}

[RootSignature(SkinBase_RootSig)]
float4 main(VSOutput input) : SV_Target0
{
	float3 objSpaceNormal = normalize(texNormal.Sample(sampler0, input.uv).xyz * 2.0 - 1.0);

	float3 worldSpaceNormal = GetWorldSpaceNormal(objSpaceNormal, input);
	float3 normal = worldSpaceNormal;

	float3 view = normalize(input.view);

	// Fetch albedo, specular parameters and static ambient occlusion:
	float3 albedo = texDiffuse.Sample(sampler0, input.uv);
	float3 specularAO = texSpecular.Sample(sampler0, input.uv).rgb;

	float occlusion = specularAO.r;
	float intensity = specularAO.b * 1.0;// specularIntensity;
	float roughness = (specularAO.g / 0.3) * 0.3;// specularRoughness;

	// Initialize the output:
	float4 color = float4(0.0, 0.0, 0.0, 0.0);
	float4 specularColor = float4(0.0, 0.0, 0.0, 0.0);

	//*
	[unroll]
	for (int i = 0; i < LightCount.x; i++)
	{
		int Index = LightIndex[i];
		float3 LColor = LightColor[Index].xyz;
		float3 light = LightPosition[Index].xyz - input.worldPosition;
		float distanceSqr = dot(light, light);
		float3 L = light * rsqrt(distanceSqr);

//		float spot = dot(lights[i].direction, -light);
//		[flatten]
//		if (spot > lights[i].falloffStart) {
//			// Calculate attenuation:
//			float curve = min(pow(dist / lights[i].farPlane, 6.0), 1.0);
//			float attenuation = lerp(1.0 / (1.0 + lights[i].attenuation * dist * dist), 0.0, curve);
		float attenuation = 1.0 / (distanceSqr + 1.0);
//
//			// And the spot light falloff:
//			spot = saturate((spot - lights[i].falloffStart) / lights[i].falloffWidth);
//
//			// Calculate some terms we will use later on:
		float3 f1 = LColor * attenuation;
		float3 f2 = albedo.rgb * f1;

		// Calculate the diffuse and specular lighting:
		float3 diffuse = saturate(dot(L, normal));
		float specular = intensity * SpecularKSK(texBeckmann, normal, L, view, roughness);

		// And also the shadowing:
		float shadow = 1.0;// ShadowPCF(input.worldPosition, i, 3, 1.0);

		// Add the diffuse and specular components:
//#ifdef SEPARATE_SPECULARS
//		color.rgb += shadow * f2 * diffuse;
//		specularColor.rgb += shadow * f1 * specular;
//#else
		color.rgb += shadow * (f2 * diffuse + f1 * specular);
//#endif

		// Add the transmittance component:
		//if (sssEnabled && translucencyEnabled)
		//	color.rgb += f2 * albedo.a * SSSSTransmittance(translucency, sssWidth, input.worldPosition, input.normal, light, shadowMaps[i], lights[i].viewProjection, lights[i].farPlane);
//		}
	}
	//*/

	// Add the ambient component:
	//color.rgb += occlusion * ambient * albedo.rgb * irradianceTex.Sample(LinearSampler, normal).rgb;
	color.rgb += occlusion * albedo.rgb * 0.6;

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
	return color;
}
