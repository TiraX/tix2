#include "S_SkyAtomsphere.h"
#include "S_SkyAtmosphereRS.hlsli"

Texture2D<float4> TransmittanceLut : register(t0);
Texture2D<float4> SkyViewLut : register(t1);
Texture3D<float4> CameraAerialPerspectiveVolumeLut : register(t2);

SamplerState sampler0 : register(s0);


float3 GetAtmosphereTransmittance(
	float3 WorldPos, float3 WorldDir, float BottomRadius, float TopRadius,
	Texture2D<float4> TransmittanceLutTexture, SamplerState TransmittanceLutTextureSampler)
{
	// For each view height entry, transmittance is only stored from zenith to horizon. Earth shadow is not accounted for.
	// It does not contain earth shadow in order to avoid texel linear interpolation artefact when LUT is low resolution.
	// As such, at the most shadowed point of the LUT when close to horizon, pure black with earth shadow is never hit.
	// That is why we analytically compute the virtual planet shadow here.
	const float2 Sol = RayIntersectSphere(WorldPos, WorldDir, float4(float3(0.0f, 0.0f, 0.0f), BottomRadius));
	if (Sol.x > 0.0f || Sol.y > 0.0f)
	{
		return 0.0f;
	}

	const float PHeight = length(WorldPos);
	const float3 UpVector = WorldPos / PHeight;
	const float LightZenithCosAngle = dot(WorldDir, UpVector);
	float2 TransmittanceLutUv;
	getTransmittanceLutUvs(PHeight, LightZenithCosAngle, TransmittanceLutUv);
	const float3 TransmittanceToLight = TransmittanceLutTexture.SampleLevel(TransmittanceLutTextureSampler, TransmittanceLutUv, 0.0f).rgb;
	return TransmittanceToLight;
}

float3 GetLightDiskLuminance(
	float3 WorldPos, float3 WorldDir, float BottomRadius, float TopRadius,
	Texture2D<float4> TransmittanceLutTexture, SamplerState TransmittanceLutTextureSampler,
	float3 AtmosphereLightDirection, float AtmosphereLightDiscCosHalfApexAngle, float3 AtmosphereLightDiscLuminance)
{
	const float ViewDotLight = dot(WorldDir, AtmosphereLightDirection);
	const float CosHalfApex = AtmosphereLightDiscCosHalfApexAngle;
	if (ViewDotLight > CosHalfApex)
	{
		const float3 TransmittanceToLight = GetAtmosphereTransmittance(
			WorldPos, WorldDir, BottomRadius, TopRadius, TransmittanceLutTexture, TransmittanceLutTextureSampler);

		return TransmittanceToLight * AtmosphereLightDiscLuminance;
	}
	return 0.0f;
}

float3 GetLightDiskLuminance(float3 WorldPos, float3 WorldDir, uint LightIndex)
{
	float t = RaySphereIntersectNearest(WorldPos, WorldDir, float3(0.0f, 0.0f, 0.0f), RadiusRange.y);
	if (t < 0.0f// No intersection with the planet
		)
	{
		float3 LightDiskLuminance = GetLightDiskLuminance(
			WorldPos, WorldDir, RadiusRange.y, RadiusRange.x,
			TransmittanceLut, sampler0,
			View_AtmosphereLightDirection0.xyz, View_AtmosphereLightDiscCosHalfApexAngle0.x, View_AtmosphereLightDiscLuminance0.xyz);

		// Clamp to avoid crazy high values (and exposed 64000.0f luminance is already crazy high, solar system sun is 1.6x10^9). Also this removes +inf float and helps TAA.
		const float3 MaxLightLuminance = 64000.0f;
		float3 ExposedLightLuminance = LightDiskLuminance * Params1.y;
		ExposedLightLuminance = min(ExposedLightLuminance, MaxLightLuminance);

		const float ViewDotLight = dot(WorldDir, View_AtmosphereLightDirection0.xyz);
		const float CosHalfApex = View_AtmosphereLightDiscCosHalfApexAngle0.x;
		const float HalfCosHalfApex = CosHalfApex + (1.0f - CosHalfApex) * 0.25; // Start fading when at 75% distance from light disk center (in cosine space)

		// Apply smooth fading at edge. This is currently an eye balled fade out that works well in many cases.
		const float Weight = 1.0 - saturate((HalfCosHalfApex - ViewDotLight) / (HalfCosHalfApex - CosHalfApex));
		ExposedLightLuminance = ExposedLightLuminance * Weight;

		return ExposedLightLuminance;
	}
	return 0.0f;
}

const static float Max10BitsFloat = 64512.0f;
float4 PrepareOutput(float3 Luminance, float3 Transmittance = float3(1.0f, 1.0f, 1.0f))
{
	const float GreyScaleTransmittance = dot(Transmittance, float3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f));
	return float4(min(Luminance, Max10BitsFloat.xxx), GreyScaleTransmittance);
}

[RootSignature(Sky_RootSig)]
float4 main(PixelShaderInput input) : SV_Target0
{
	float2 UvBuffer = input.uv;// PixPos* View_BufferSizeAndInvSize.zw;
	float4 SVPos = float4(input.uv * ViewSizeAndInv.xy, SkyWorldCameraOrigin.w, 1.f);

	float3 WorldPos = GetCameraPlanetPos();
	float3 WorldDir = GetScreenWorldDir(SVPos);

	float3 PreExposedL = GetLightDiskLuminance(WorldPos, WorldDir, 0);
	float3 LuminanceScale = SkyLuminanceFactor.xyz;

	float ViewHeight = length(WorldPos);

	//if (ViewHeight < Atmosphere_TopRadiusKm)
	{
		float2 UV;

		float3x3 LocalReferencial = GetSkyViewLutReferential(WorldPos, ViewForward.xyz, ViewRight.xyz);


		float3 WorldPosLocal = float3(0.0, 0.0, ViewHeight);
		float3 UpVectorLocal = float3(0.0, 0.0, 1.0);
		float3 WorldDirLocal = mul(WorldDir, LocalReferencial);


		float ViewZenithCosAngle = dot(WorldDirLocal, UpVectorLocal);
		bool IntersectGround = RaySphereIntersectNearest(WorldPosLocal, WorldDirLocal, float3(0, 0, 0), RadiusRange.y) >= 0.0f;

		SkyViewLutParamsToUv(IntersectGround, ViewZenithCosAngle, WorldDirLocal, ViewHeight, RadiusRange.y, SkyViewLutSizeAndInv, UV);
		float3 SkyLuminance = SkyViewLut.SampleLevel(sampler0, UV, 0).rgb;

		PreExposedL += SkyLuminance * LuminanceScale;
		return PrepareOutput(PreExposedL);
	}
	//float4 sky = SkyViewLut.Sample(sampler0, input.uv);
	//return sky;// float4(input.uv, 0.0, 1.0);
}
