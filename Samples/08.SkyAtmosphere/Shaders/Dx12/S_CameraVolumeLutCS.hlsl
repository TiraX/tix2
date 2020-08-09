
#include "S_SkyAtomsphere.h"

#define CameraVolumeLut_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1))," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_CLAMP, " \
                      "addressV = TEXTURE_ADDRESS_CLAMP, " \
                      "addressW = TEXTURE_ADDRESS_CLAMP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR)"

Texture2D<float4> TransmittanceLut : register(t0);
Texture2D<float4> MultiScatteredLuminanceLut : register(t1);
RWTexture3D<float4> CameraAerialPerspectiveVolume : register(u0);
SamplerState LinearSampler : register(s0);

SingleScatteringResult IntegrateSingleScatteredLuminance(
	in float4 SVPos, in float3 WorldPos, in float3 WorldDir,
	in bool Ground, in SamplingSetup Sampling, in float DeviceZ, in bool MieRayPhase,
	in float3 Light0Dir, in float3 Light1Dir, in float3 Light0Illuminance, in float3 Light1Illuminance,
	in float AerialPespectiveViewDistanceScale,
	in float tMaxMax = 9000000.f)
{
	SingleScatteringResult Result;
	Result.L = 0;
	Result.OpticalDepth = 0;
	Result.Transmittance = 1.f;
	Result.MultiScatAs1 = 0;

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
		float3 TransmittanceToLight0 = GetWhiteTransmittance();
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
		float3 TransmittanceToLight0 = GetWhiteTransmittance();

		const float NdotL0 = saturate(dot(UpVector, Light0Dir));
		L += Light0Illuminance * TransmittanceToLight0 * Throughput * NdotL0 * GroundAlbedo.rgb / PI;
	}

	Result.L = L;
	Result.OpticalDepth = OpticalDepth;
	Result.Transmittance = Throughput * PlanetOnOpaque;

	return Result;
}

bool MoveToTopAtmosphere(inout float3 WorldPos, in float3 WorldDir, in float AtmosphereTopRadius)
{
	float ViewHeight = length(WorldPos);
	if (ViewHeight > AtmosphereTopRadius)
	{
		float TTop = RaySphereIntersectNearest(WorldPos, WorldDir, float3(0.0f, 0.0f, 0.0f), AtmosphereTopRadius);
		if (TTop >= 0.0f)
		{
			float3 UpVector = WorldPos / ViewHeight;
			float3 UpOffset = UpVector * -0.001f;
			WorldPos = WorldPos + WorldDir * TTop + UpOffset;
		}
		else
		{

			return false;
		}
	}
	return true;
}

[RootSignature(CameraVolumeLut_RootSig)]
[numthreads(4, 4, 4)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
	float2 PixPos = float2(dispatchThreadId.xy) + 0.5f;
	float2 UV = PixPos * CameraAerialPerspectiveVolumeSizeAndInv.zw;

	float4 SVPos = float4(UV * ViewSizeAndInv.xy, 0.0f, 1.0f);// SV_POS as if resolution was the one from the scene view.
	float3 WorldDir = GetScreenWorldDir(SVPos);
	float3 CamPos = GetCameraPlanetPos();

	float Slice = ((float(dispatchThreadId.z) + 0.5f) * CameraAerialPerspectiveVolumeDepthInfo.y); // +0.5 to always have a distance to integrate over
	Slice *= Slice;	// squared distribution
	Slice *= CameraAerialPerspectiveVolumeDepthInfo.x;

	float3 RayStartWorldPos = CamPos + AerialPerspectiveInfo.x * WorldDir; // Offset according to start depth
	float ViewHeight;

	// Compute position from froxel information
	float tMax = Slice * CameraAerialPerspectiveVolumeDepthInfo.z;
	float3 VoxelWorldPos = RayStartWorldPos + tMax * WorldDir;
	float VoxelHeight = length(VoxelWorldPos);

	// Check if the voxel is under the horizon.
	const float UnderGround = VoxelHeight < RadiusRange.y;

	// Check if the voxel is beind the planet (to next check for below the horizon case)
	float3 CameraToVoxel = VoxelWorldPos - CamPos;
	float CameraToVoxelLen = length(CameraToVoxel);
	float3 CameraToVoxelDir = CameraToVoxel / CameraToVoxelLen;
	float PlanetNearT = RaySphereIntersectNearest(CamPos, CameraToVoxelDir, float3(0, 0, 0), RadiusRange.y);
	bool BelowHorizon = PlanetNearT > 0.0f && CameraToVoxelLen > PlanetNearT;

	if (BelowHorizon || UnderGround)
	{
		CamPos += normalize(CamPos) * 0.02f;	// TODO: investigate why we need this workaround. Without it, we get some bad color and flickering on the ground only (floating point issue with sphere intersection code?).

		float3 VoxelWorldPosNorm = normalize(VoxelWorldPos);
		float3 CamProjOnGround = normalize(CamPos) * RadiusRange.y;
		float3 VoxProjOnGround = VoxelWorldPosNorm * RadiusRange.y;
		float3 VoxelGroundToRayStart = CamPos - VoxProjOnGround;
		if (BelowHorizon && dot(normalize(VoxelGroundToRayStart), VoxelWorldPosNorm) < 0.0001f)
		{
			// We are behing the sphere and the psehre normal is pointing away from V: we are below the horizon.
			float3 MiddlePoint = 0.5f * (CamProjOnGround + VoxProjOnGround);
			float MiddlePointHeight = length(MiddlePoint);

			// Compute the new position to evaluate and store the value in the voxel.
			// the position is the oposite side of the horizon point from the view point,
			// The offset of 1.001f is needed to get matching colors and for the ray to not hit the earth again later due to floating point accuracy
			float3 MiddlePointOnGround = normalize(MiddlePoint) * RadiusRange.y;// *1.001f;
			VoxelWorldPos = CamPos + 2.0f * (MiddlePointOnGround - CamPos);

			//CameraAerialPerspectiveVolumeUAV[ThreadId] = float4(1, 0, 0, 0);
			//return; // debug
		}
		else if (UnderGround)
		{
			//No obstruction from the planet, so use the point on the ground
			VoxelWorldPos = normalize(VoxelWorldPos) * (RadiusRange.y);
			//VoxelWorldPos = CamPos + CameraToVoxelDir * PlanetNearT;		// better match but gives visual artefact as visible voxels on a simple plane at altitude 0

			//CameraAerialPerspectiveVolumeUAV[ThreadId] = float4(0, 1, 0, 0);
			//return; // debug
		}

		WorldDir = normalize(VoxelWorldPos - CamPos);
		RayStartWorldPos = CamPos + AerialPerspectiveInfo.x * WorldDir; // Offset according to start depth
		tMax = length(VoxelWorldPos - RayStartWorldPos);
	}
	float tMaxMax = tMax;

	// Move ray marching start up to top atmosphere.
	ViewHeight = length(RayStartWorldPos);
	if (ViewHeight >= RadiusRange.x)
	{
		float3 prevWorlPos = RayStartWorldPos;
		if (!MoveToTopAtmosphere(RayStartWorldPos, WorldDir, RadiusRange.x))
		{
			// Ray is not intersecting the atmosphere
			CameraAerialPerspectiveVolume[dispatchThreadId] = float4(0.0f, 0.0f, 0.0f, 1.0f);
			return;
		}
		float LengthToAtmosphere = length(prevWorlPos - RayStartWorldPos);
		if (tMaxMax < LengthToAtmosphere)
		{
			// tMaxMax for this voxel is not within the planet atmosphere
			CameraAerialPerspectiveVolume[dispatchThreadId] = float4(0.0f, 0.0f, 0.0f, 1.0f);
			return;
		}
		// Now world position has been moved to the atmosphere boundary: we need to reduce tMaxMax accordingly. 
		tMaxMax = max(0.0, tMaxMax - LengthToAtmosphere);
	}


	SamplingSetup Sampling;
	{
		Sampling.VariableSampleCount = false;
		Sampling.SampleCountIni = max(1.0f, (float(dispatchThreadId.z) + 1.0f) * AerialPerspectiveInfo.y);
	}
	const bool Ground = false;
	const float DeviceZ = FarDepthValue;
	const bool MieRayPhase = true;
	//float AerialPespectiveViewDistanceScale = AerialPespectiveViewDistanceScale;
	SingleScatteringResult ss = IntegrateSingleScatteredLuminance(
		float4(PixPos, 0.0f, 1.0f), RayStartWorldPos, WorldDir,
		Ground, Sampling, DeviceZ, MieRayPhase,
		View_AtmosphereLightDirection0.xyz, View_AtmosphereLightDirection1.xyz, View_AtmosphereLightColor0.rgb, View_AtmosphereLightColor1.rgb,
		AerialPerspectiveInfo.z,
		tMaxMax);

	const float Transmittance = dot(ss.Transmittance, float3(1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f));
	CameraAerialPerspectiveVolume[dispatchThreadId] = float4(ss.L, Transmittance);
}
