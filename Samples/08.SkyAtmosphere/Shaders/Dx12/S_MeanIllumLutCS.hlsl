#include "S_SkyAtomsphere.h"

#define MeanIllumLut_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=1), UAV(u0, numDescriptors=1))," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR)"


Texture2D<float4> TransmittanceLut : register(t0);
RWTexture2D<float4> MultiScatteredLuminanceLut : register(u0); 
SamplerState LinearSampler : register(s0);

float3 GetTransmittance(in float LightZenithCosAngle, in float PHeight)
{
    float2 UV;
    LutTransmittanceParamsToUv(PHeight, LightZenithCosAngle, UV);
    float3 TransmittanceToLight = TransmittanceLut.SampleLevel(LinearSampler, UV, 0).rgb;
    return TransmittanceToLight;
}

SingleScatteringResult IntegrateSingleScatteredLuminance(
	in float4 SVPos, in float3 WorldPos, in float3 WorldDir,
	in bool Ground, in SamplingSetup Sampling, in float DeviceZ, in bool MieRayPhase,
	in float3 Light0Dir, in float3 Light1Dir, in float3 Light0Illuminance, in float3 Light1Illuminance,
	in float AerialPespectiveViewDistanceScale,
	in float tMaxMax = 9000000.f)
{
	SingleScatteringResult Result;
	Result.L = float3(0.f, 0.f, 0.f);
	Result.OpticalDepth = float3(0.f, 0.f, 0.f);
	Result.Transmittance = float3(1.f, 1.f, 1.f);
	Result.MultiScatAs1 = float3(0.f, 0.f, 0.f);

	if (dot(WorldPos, WorldPos) <= RadiusRange.w)
	{
		return Result;	// Camera is inside the planet ground
	}

	float2 PixPos = SVPos.xy;

	// Compute next intersection with atmosphere or ground
	float3 PlanetO = float3(0.f, 0.f, 0.f);
	float tBottom = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, RadiusRange.y);
	float tTop = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, RadiusRange.x);
	float tMax = 0.f;
	if (tBottom < 0.f)
	{
		if (tTop < 0.f)
		{
			tMax = 0.f;	// No intersection with planet nor its atmosphere: stop right away
			return Result;
		}
		else
		{
			tMax = tTop;
		}
	}
	else
	{
		if (tTop > 0.f)
		{
			tMax = min(tTop, tBottom);
		}
	}

	float PlanetOnOpaque = 1.f;	// This is used to hide opaque meshes under the planet ground
	tMax = min(tMax, tMaxMax);

	// Sample Count
	float SampleCount = Sampling.SampleCountIni;
	float SampleCountFloor = Sampling.SampleCountIni;
	float tMaxFloor = tMax;
	if (Sampling.VariableSampleCount)
	{
		SampleCount = lerp(Sampling.MinSampleCount, Sampling.MaxSampleCount, saturate(tMax * Sampling.DistanceToSampleCountMaxInv));
		SampleCountFloor = floor(SampleCount);
		tMaxFloor = tMax * SampleCountFloor / SampleCount;	// rescale tMax to map to the last entire step segment
	}
	float dt = tMax / SampleCount;

	// Phase functions
	const float uniformPhase = 1.f / (4.f * PI);
	const float3 wi = Light0Dir;
	const float3 wo = WorldDir;
	float cosTheta = dot(wi, wo);
	//float MiePhaseValueLight0 = HgPhase(MiePhaseG, -cosTheta);
	float MiePhaseValueLight0 = HgPhase(MieRayleigh.x, -cosTheta);
	float RayleighPhaseValueLight0 = RayleighPhase(cosTheta);

	// Ray march the atmosphere to integrate optical depth
	float3 L = 0.f;
	float3 Throughput = 1.f;
	float3 OpticalDepth = 0.f;
	float t = 0.f;
	float tPrev = 0.f;

	float3 ExposedLight0Illuminance = Light0Illuminance * Params1.y;// ViewPreExposure;

	float PixelNoise = DEFAULT_SAMPLE_OFFSET;
	for (float SampleI = 0.f; SampleI < SampleCount; SampleI += 1.f)
	{
		// Compute Current ray t and sample point P
		if (Sampling.VariableSampleCount)
		{
			// More expenssive but artefact free
			float t0 = (SampleI) / SampleCountFloor;
			float t1 = (SampleI + 1.f) / SampleCountFloor;

			// Non linear distribution of samples within the range
			t0 = t0 * t0;
			t1 = t1 * t1;

			// Make t0 and t1 world space distance
			t0 = tMaxFloor * t0;
			if (t1 > 1.f)
			{
				t1 = tMax;
			}
			else
			{
				t1 = tMaxFloor * t1;
			}
			t = t0 + (t1 - t0) * PixelNoise;
			dt = t1 - t0;
		}
		else
		{
			t = tMax * (SampleI + PixelNoise) / SampleCount;
		}
		float3 P = WorldPos + t * WorldDir;
		float PHeight = length(P);

		// Sample the medium
		MediumSampleRGB Medium = SampleMediumRGB(P);
		const float3 SampleOpticalDepth = Medium.Extinction * dt * AerialPespectiveViewDistanceScale;
		const float3 SampleTransmittance = exp(-SampleOpticalDepth);
		OpticalDepth += SampleOpticalDepth;

		// Phase and transmittance for light0
		const float3 UpVector = P / PHeight;
		float Light0ZenithCosAngle = dot(Light0Dir, UpVector);
		float3 TransmittanceToLight0 = GetTransmittance(Light0ZenithCosAngle, PHeight);
		float3 PhaseTimesScattering0;
		if (MieRayPhase)
		{
			PhaseTimesScattering0 = Medium.ScatteringMie * MiePhaseValueLight0 + Medium.ScatteringRay * RayleighPhaseValueLight0;
		}
		else
		{
			PhaseTimesScattering0 = Medium.Scattering * uniformPhase;
		}

		// Multiple scattering approximation
		float3 MultiScatteredLuminance0 = 0.f;

		// Planet shadow
		float tPlanet0 = RaySphereIntersectNearest(P, Light0Dir, PlanetO + 0.001f * UpVector, RadiusRange.y);
		float PlanetShadow0 = tPlanet0 >= 0.f ? 0.f : 1.f;
		// MultiScatteredLuminance is already pre-exposed, atmospheric light contribution needs to be pre exposed
		// Multi-scattering is also not affected by PlanetShadow or TransmittanceToLight because it contains diffuse light after single scattering.
		float3 S = ExposedLight0Illuminance * (PlanetShadow0 * TransmittanceToLight0 * PhaseTimesScattering0 + MultiScatteredLuminance0 * Medium.Scattering);

		// When using the power serie to accumulate all sattering order, serie r must be <1 for a serie to converge. 
		// Under extreme coefficient, MultiScatAs1 can grow larger and thus results in broken visuals. 
		// The way to fix that is to use a proper analytical integration as porposed in slide 28 of http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/ 
		// However, it is possible to disable as it can also work using simple power serie sum unroll up to 5th order. The rest of the orders has a really low contribution. 

		// 1 is the integration of luminance over the 4pi of a sphere, and assuming an isotropic phase function of 1.0/(4*PI) 
		Result.MultiScatAs1 += Throughput * Medium.Scattering * 1.0f * dt;

		// See slide 28 at http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/ 
		float3 Sint = (S - S * SampleTransmittance) / Medium.Extinction;	// integrate along the current step segment 
		L += Throughput * Sint;														// accumulate and also take into account the transmittance from previous steps
		Throughput *= SampleTransmittance;

		tPrev = t;
	}

	if (Ground && tMax == tBottom)
	{
		// Account for bounced light off the planet
		float3 P = WorldPos + tBottom * WorldDir;
		float PHeight = length(P);

		const float3 UpVector = P / PHeight;
		float Light0ZenithCosAngle = dot(Light0Dir, UpVector);
		float3 TransmittanceToLight0 = GetTransmittance(Light0ZenithCosAngle, PHeight);

		const float NdotL0 = saturate(dot(UpVector, Light0Dir));
		L += Light0Illuminance * TransmittanceToLight0 * Throughput * NdotL0 * GroundAlbedo.rgb / PI;
	}

	Result.L = L;
	Result.OpticalDepth = OpticalDepth;
	Result.Transmittance = Throughput * PlanetOnOpaque;

	return Result;
}

[RootSignature(MeanIllumLut_RootSig)]
[numthreads(8, 8, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	float2 PixPos = float2(dispatchThreadId.xy) + 0.5f;

    float CosLightZenithAngle = (PixPos.x * MultiScatteredLuminanceLutSizeAndInv.z) * 2.f - 1.f;
    float3 LightDir = float3(0.f, sqrt(saturate(1.f - CosLightZenithAngle * CosLightZenithAngle)), CosLightZenithAngle);
    const float3 NullLightDirection = float3(0.f, 0.f, 1.f);
    const float3 NullLightIlluminance = float3(0.f, 0.f, 0.f);
    const float3 OneIlluminance = float3(1.f, 1.f, 1.f);
    float ViewHeight = RadiusRange.y + (PixPos.y * MultiScatteredLuminanceLutSizeAndInv.w) * (RadiusRange.x - RadiusRange.y);

    float3 WorldPos = float3(0.0f, 0.0f, ViewHeight);
    float3 WorldDir = float3(0.0f, 0.0f, 1.0f);

    SamplingSetup Sampling;
    {
        Sampling.VariableSampleCount = false;
        Sampling.SampleCountIni = Params1.w;// MultiScatteringSampleCount;
    }
    const bool Ground = true;
    const float DeviceZ = 0.f;
    const bool MieRayPhase = false;
    const float AerialPespectiveViewDistanceScale = 1.f;

    const float SphereSolidAngle = 4.f * PI;
    const float IsotropicPhase = 1.f / SphereSolidAngle;
    SingleScatteringResult r0 = IntegrateSingleScatteredLuminance(float4(PixPos, 0.f, 1.f), WorldPos, WorldDir, Ground, Sampling, DeviceZ, MieRayPhase,
        LightDir, NullLightDirection, OneIlluminance, NullLightIlluminance, AerialPespectiveViewDistanceScale);
    SingleScatteringResult r1 = IntegrateSingleScatteredLuminance(float4(PixPos, 0.f, 1.f), WorldPos, -WorldDir, Ground, Sampling, DeviceZ, MieRayPhase,
        LightDir, NullLightDirection, OneIlluminance, NullLightIlluminance, AerialPespectiveViewDistanceScale);

    float3 IntegratedIlluminance = (SphereSolidAngle / 2.f) * (r0.L + r1.L);
    float3 MultiScatAs1 = (1.f / 2.f) * (r0.MultiScatAs1 + r1.MultiScatAs1);
    float3 InScatteredLuminance = IntegratedIlluminance * IsotropicPhase;
    float3 MultiScatAs1SQR = MultiScatAs1 * MultiScatAs1;
    float3 L = InScatteredLuminance * (1.f + MultiScatAs1 + MultiScatAs1SQR + MultiScatAs1 * MultiScatAs1SQR + MultiScatAs1SQR * MultiScatAs1SQR);
    MultiScatteredLuminanceLut[dispatchThreadId.xy] = float4(L * MieRayleigh.w, 1.f);
}
