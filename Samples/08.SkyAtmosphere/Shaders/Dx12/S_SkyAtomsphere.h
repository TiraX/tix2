//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************


cbuffer FAtmosphereParam : register(b0)
{
    float4 TransmittanceLutSizeAndInv;  // xy = Size; zw = InvSize;
	float4 MultiScatteredLuminanceLutSizeAndInv;
	float4 SkyViewLutSizeAndInv;
	float4 CameraAerialPerspectiveVolumeSizeAndInv;
	float4 ViewSizeAndInv;

	// x = CameraAerialPerspectiveVolumeDepthResolution; y = CameraAerialPerspectiveVolumeDepthResolutionInv
	// z = CameraAerialPerspectiveVolumeDepthSliceLengthKm; w = CameraAerialPerspectiveVolumeDepthSliceLengthKmInv;
	float4 CameraAerialPerspectiveVolumeDepthInfo;
	float4 AerialPerspectiveInfo;	// x = AerialPerspectiveStartDepthKm; y = CameraAerialPerspectiveSampleCountPerSlice; z = AerialPespectiveViewDistanceScale;
	//float AerialPerspectiveStartDepthKm;
	//float CameraAerialPerspectiveSampleCountPerSlice;
	//float AerialPespectiveViewDistanceScale;

    float4 RadiusRange;		// x = TopRadiusKm; y = BottomRadiusKm; z = sqrt(x * x - y * y); w = y * y;
	float4 MieRayleigh;		// x = MiePhaseG; y = MieDensityExpScale; z = RayleighDensityExpScale; w = MultiScatteringFactor;
	//float MiePhaseG;
	//float MieDensityExpScale;
	//float RayleighDensityExpScale;
	//float MultiScatteringFactor;
	float4 Params1;		// x = TransmittanceSampleCount; y = ViewPreExposure; z = AbsorptionDensity0LayerWidth; w = MultiScatteringSampleCount
	//float TransmittanceSampleCount;
	//float ViewPreExposure;
	//float AbsorptionDensity0LayerWidth;
	//float MultiScatteringSampleCount;
	float4 AbsorptionDensity01MA;
	float4 MieScattering;
	float4 MieAbsorption;
	float4 MieExtinction;
	float4 RayleighScattering;
	float4 AbsorptionExtinction;
	float4 GroundAlbedo;

	float4 AtmosphereLightDirection0;
	float4 AtmosphereLightDirection1;
	float4 AtmosphereLightColor0;
	float4 AtmosphereLightColor1;
	float4 SkyLuminanceFactor;
	float4 DistantSkyLightSampleAltitude;

	float4 ViewForward;
	float4 ViewRight;
	float4 SkySampleParam;	// x = FastSkySampleCountMin; y = FastSkySampleCountMax; z = FastSkyDistanceToSampleCountMaxInv; w = 1;
	//float FastSkySampleCountMin;
	//float FastSkySampleCountMax;
	//float FastSkyDistanceToSampleCountMaxInv;
	//float StartDepthZ;
	float4 SkyWorldCameraOrigin;	// perframe, should be isolated; xyz = origin; w = StartDepthZ
	float4 SkyPlanetCenterAndViewHeight;
	float4 View_AtmosphereLightDirection0;
	float4 View_AtmosphereLightDirection1;
	float4 View_AtmosphereLightColor0;
	float4 View_AtmosphereLightColor1;
	float4 View_AtmosphereLightDiscLuminance0;
	float4 View_AtmosphereLightDiscCosHalfApexAngle0;
	float4x4 SVPositionToTranslatedWorld;
	float4x4 ScreenToWorld;
};

static const float PI = 3.14159f;

////////////////////////////////////////////////////////////
// LUT functions
////////////////////////////////////////////////////////////

float2 FromUnitToSubUvs(float2 uv, float4 SizeAndInvSize) { return (uv + 0.5f * SizeAndInvSize.zw) * (SizeAndInvSize.xy / (SizeAndInvSize.xy + 1.0f)); }
float2 FromSubUvsToUnit(float2 uv, float4 SizeAndInvSize) { return (uv - 0.5f * SizeAndInvSize.zw) * (SizeAndInvSize.xy / (SizeAndInvSize.xy - 1.0f)); }

// Transmittance LUT function parameterisation from Bruneton 2017 https://github.com/ebruneton/precomputed_atmospheric_scattering
// uv in [0,1]
// ViewZenithCosAngle in [-1,1]
// ViewHeight in [bottomRAdius, topRadius]

void UvToLutTransmittanceParams(out float ViewHeight, out float ViewZenithCosAngle, in float2 UV)
{
	float Xmu = UV.x;
	float Xr = UV.y;

    float H = RadiusRange.z;
	float Rho = H * Xr;
	ViewHeight = sqrt(Rho * Rho + RadiusRange.w);

	float Dmin = RadiusRange.x - ViewHeight;
	float Dmax = Rho + H;
	float D = Dmin + Xmu * (Dmax - Dmin);
	ViewZenithCosAngle = D == 0.0f ? 1.0f : (H * H - Rho * Rho - D * D) / (2.0f * ViewHeight * D);
	ViewZenithCosAngle = clamp(ViewZenithCosAngle, -1.0f, 1.0f);
}

// 4th order polynomial approximation
// 4 VGRP, 16 ALU Full Rate
// 7 * 10^-5 radians precision
// Reference : Handbook of Mathematical Functions (chapter : Elementary Transcendental Functions), M. Abramowitz and I.A. Stegun, Ed.
float acosFast4(float inX)
{
	float x1 = abs(inX);
	float x2 = x1 * x1;
	float x3 = x2 * x1;
	float s;

	s = -0.2121144f * x1 + 1.5707288f;
	s = 0.0742610f * x2 + s;
	s = -0.0187293f * x3 + s;
	s = sqrt(1.0f - x1) * s;

	// acos function mirroring
	// check per platform if compiles to a selector - no branch neeeded
	return inX >= 0.0f ? s : PI - s;
}

// max absolute error 1.3x10^-3
// Eberly's odd polynomial degree 5 - respect bounds
// 4 VGPR, 14 FR (10 FR, 1 QR), 2 scalar
// input [0, infinity] and output [0, PI/2]
float atanFastPos(float x)
{
	float t0 = (x < 1.0f) ? x : 1.0f / x;
	float t1 = t0 * t0;
	float poly = 0.0872929f;
	poly = -0.301895f + poly * t1;
	poly = 1.0f + poly * t1;
	poly = poly * t0;
	return (x < 1.0f) ? poly : (0.5 * PI) - poly;
}

// 4 VGPR, 16 FR (12 FR, 1 QR), 2 scalar
// input [-infinity, infinity] and output [-PI/2, PI/2]
float atanFast(float x)
{
	float t0 = atanFastPos(abs(x));
	return (x < 0) ? -t0 : t0;
}

float atan2Fast(float y, float x)
{
	float t0 = max(abs(x), abs(y));
	float t1 = min(abs(x), abs(y));
	float t3 = t1 / t0;
	float t4 = t3 * t3;

	// Same polynomial as atanFastPos
	t0 = +0.0872929;
	t0 = t0 * t4 - 0.301895;
	t0 = t0 * t4 + 1.0;
	t3 = t0 * t3;

	t3 = abs(y) > abs(x) ? (0.5 * PI) - t3 : t3;
	t3 = x < 0 ? PI - t3 : t3;
	t3 = y < 0 ? -t3 : t3;

	return t3;
}

void getTransmittanceLutUvs(
	in float viewHeight, in float viewZenithCosAngle,
	out float2 UV)
{
	float H = RadiusRange.z;
	float Rho = sqrt(max(0.0f, viewHeight * viewHeight - RadiusRange.w));

	float Discriminant = viewHeight * viewHeight * (viewZenithCosAngle * viewZenithCosAngle - 1.0f) + RadiusRange.x * RadiusRange.x;
	float D = max(0.0f, (-viewHeight * viewZenithCosAngle + sqrt(Discriminant)));

	float Dmin = RadiusRange.x - viewHeight;
	float Dmax = Rho + H;
	float Xmu = (D - Dmin) / (Dmax - Dmin);
	float Xr = Rho / H;

	UV = float2(Xmu, Xr);

}

void SkyViewLutParamsToUv(
	in bool IntersectGround, in float ViewZenithCosAngle, in float3 ViewDir, in float ViewHeight, in float BottomRadius, in float4 SkyViewLutSizeAndInvSize,
	out float2 UV)
{
	float Vhorizon = sqrt(ViewHeight * ViewHeight - BottomRadius * BottomRadius);
	float CosBeta = Vhorizon / ViewHeight;				// GroundToHorizonCos
	float Beta = acosFast4(CosBeta);
	float ZenithHorizonAngle = PI - Beta;
	float ViewZenithAngle = acosFast4(ViewZenithCosAngle);

	if (!IntersectGround)
	{
		float Coord = ViewZenithAngle / ZenithHorizonAngle;
		Coord = 1.0f - Coord;
		Coord = sqrt(Coord);
		Coord = 1.0f - Coord;
		UV.y = Coord * 0.5f;
	}
	else
	{
		float Coord = (ViewZenithAngle - ZenithHorizonAngle) / Beta;
		Coord = sqrt(Coord);
		UV.y = Coord * 0.5f + 0.5f;
	}

	{
		UV.x = (atan2Fast(-ViewDir.y, -ViewDir.x) + PI) / (2.0f * PI);
	}

	// Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
	UV = FromUnitToSubUvs(UV, SkyViewLutSizeAndInvSize);
}

void LutTransmittanceParamsToUv(in float ViewHeight, in float ViewZenithCosAngle, out float2 UV)
{
	getTransmittanceLutUvs(ViewHeight, ViewZenithCosAngle, UV);
}

float2 RayIntersectSphere(float3 RayOrigin, float3 RayDirection, float4 Sphere)
{
	float3 LocalPosition = RayOrigin - Sphere.xyz;
	float LocalPositionSqr = dot(LocalPosition, LocalPosition);

	float3 QuadraticCoef;
	QuadraticCoef.x = dot(RayDirection, RayDirection);
	QuadraticCoef.y = 2 * dot(RayDirection, LocalPosition);
	QuadraticCoef.z = LocalPositionSqr - Sphere.w * Sphere.w;

	float Discriminant = QuadraticCoef.y * QuadraticCoef.y - 4 * QuadraticCoef.x * QuadraticCoef.z;

	float2 Intersections = -1;


	[flatten]
	if (Discriminant >= 0)
	{
		float SqrtDiscriminant = sqrt(Discriminant);
		Intersections = (-QuadraticCoef.y + float2(-1, 1) * SqrtDiscriminant) / (2 * QuadraticCoef.x);
	}

	return Intersections;
}
// - RayOrigin: ray origin
// - RayDir: normalized ray direction
// - SphereCenter: sphere center
// - SphereRadius: sphere radius
// - Returns distance from RayOrigin to closest intersecion with sphere,
//   or -1.0 if no intersection.
float RaySphereIntersectNearest(float3 RayOrigin, float3 RayDir, float3 SphereCenter, float SphereRadius)
{
	float2 Sol = RayIntersectSphere(RayOrigin, RayDir, float4(SphereCenter, SphereRadius));
	float Sol0 = Sol.x;
	float Sol1 = Sol.y;
	if (Sol0 < 0.0f && Sol1 < 0.0f)
	{
		return -1.0f;
	}
	if (Sol0 < 0.0f)
	{
		return max(0.0f, Sol1);
	}
	else if (Sol1 < 0.0f)
	{
		return max(0.0f, Sol0);
	}
	return max(0.0f, min(Sol0, Sol1));
}

////////////////////////////////////////////////////////////
// Participating medium properties
////////////////////////////////////////////////////////////

float RayleighPhase(float CosTheta)
{
	float Factor = 3.0f / (16.0f * PI);
	return Factor * (1.0f + CosTheta * CosTheta);
}

float HgPhase(float G, float CosTheta)
{
	// Reference implementation (i.e. not schlick approximation). 
	// See http://www.pbr-book.org/3ed-2018/Volume_Scattering/Phase_Functions.html
	float Numer = 1.0f - G * G;
	float Denom = 1.0f + G * G + 2.0f * G * CosTheta;
	return Numer / (4.0f * PI * Denom * sqrt(Denom));
}

float3 GetAlbedo(float3 Scattering, float3 Extinction)
{
	return Scattering / max(0.001f, Extinction);
}

float3 GetWhiteTransmittance()
{
	return float3(1.f, 1.f, 1.f);
}

////////////////////////////////////////////////////////////
// Main scattering/transmitance integration function
////////////////////////////////////////////////////////////

struct SingleScatteringResult
{
	float3 L;						// Scattered light (luminance)
	float3 OpticalDepth;			// Optical depth (1/m)
	float3 Transmittance;			// Transmittance in [0,1] (unitless)
	float3 MultiScatAs1;
};

struct SamplingSetup
{
	bool VariableSampleCount;
	float SampleCountIni;			// Used when VariableSampleCount is false
	float MinSampleCount;
	float MaxSampleCount;
	float DistanceToSampleCountMaxInv;
};

struct MediumSampleRGB
{
	float3 Scattering;
	float3 Absorption;
	float3 Extinction;

	float3 ScatteringMie;
	float3 AbsorptionMie;
	float3 ExtinctionMie;

	float3 ScatteringRay;
	float3 AbsorptionRay;
	float3 ExtinctionRay;

	float3 ScatteringOzo;
	float3 AbsorptionOzo;
	float3 ExtinctionOzo;

	float3 Albedo;
};

// If this is changed, please also update USkyAtmosphereComponent::GetTransmittance 
MediumSampleRGB SampleMediumRGB(in float3 WorldPos)
{
	const float SampleHeight = max(0.0, (length(WorldPos) - RadiusRange.y));

	const float DensityMie = exp(MieRayleigh.y * SampleHeight);

	const float DensityRay = exp(MieRayleigh.z * SampleHeight);

	const float DensityOzo = SampleHeight < Params1.z ? //AbsorptionDensity0LayerWidth ?
		saturate(AbsorptionDensity01MA.x * SampleHeight + AbsorptionDensity01MA.y) :	// We use saturate to allow the user to create plateau, and it is free on GCN.
		saturate(AbsorptionDensity01MA.z * SampleHeight + AbsorptionDensity01MA.w);

	MediumSampleRGB s;

	s.ScatteringMie = DensityMie * MieScattering.rgb;
	s.AbsorptionMie = DensityMie * MieAbsorption.rgb;
	s.ExtinctionMie = DensityMie * MieExtinction.rgb;

	s.ScatteringRay = DensityRay * RayleighScattering.rgb;
	s.AbsorptionRay = 0.0f;
	s.ExtinctionRay = s.ScatteringRay + s.AbsorptionRay;

	s.ScatteringOzo = 0.0f;
	s.AbsorptionOzo = DensityOzo * AbsorptionExtinction.rgb;
	s.ExtinctionOzo = s.ScatteringOzo + s.AbsorptionOzo;

	s.Scattering = s.ScatteringMie + s.ScatteringRay + s.ScatteringOzo;
	s.Absorption = s.AbsorptionMie + s.AbsorptionRay + s.AbsorptionOzo;
	s.Extinction = s.ExtinctionMie + s.ExtinctionRay + s.ExtinctionOzo;
	s.Albedo = GetAlbedo(s.Scattering, s.Extinction);

	return s;
}

#define FarDepthValue 0.f
#define DEFAULT_SAMPLE_OFFSET 0.3f

#define M_TO_SKY_UNIT 0.001f

float3 GetCameraPlanetPos()
{
	return (SkyWorldCameraOrigin.xyz - SkyPlanetCenterAndViewHeight.xyz) * M_TO_SKY_UNIT;
}

float4 GetScreenWorldPos(float4 SVPos, float DeviceZ)
{

	DeviceZ = max(0.000000000001, DeviceZ);

	float4 Pos = mul(float4(SVPos.xyz, 1.f), SVPositionToTranslatedWorld);
	float3 P = Pos.xyz / Pos.w;
	return float4(P, 1.f);
}

float4 SvPositionToScreenPosition(float4 SvPosition)
{
	// todo: is already in .w or needs to be reconstructed like this:
//	SvPosition.w = ConvertFromDeviceZ(SvPosition.z);

	float2 PixelPos = SvPosition.xy;

	// NDC (NormalizedDeviceCoordinates, after the perspective divide)
	float3 NDCPos = float3((PixelPos * ViewSizeAndInv.zw - 0.5f) * float2(2, -2), SvPosition.z);

	// SvPosition.w: so .w has the SceneDepth, some mobile code and the DepthFade material expression wants that
	return float4(NDCPos.xyz, 1) * SvPosition.w;
}

float3 GetScreenWorldDir(in float4 SVPos)
{
	float2 ScreenPosition = SvPositionToScreenPosition(SVPos).xy;
	const float Depth = 1000000.0f;
	float4 WorldPos = mul(float4(ScreenPosition * Depth, Depth, 1), ScreenToWorld);
	return normalize(WorldPos.xyz - SkyWorldCameraOrigin.xyz);
}

float3x3 GetSkyViewLutReferential(in float3 WorldPos, in float3 ViewForward, in float3 ViewRight)
{
	float3 Up = normalize(WorldPos);
	float3 Forward = ViewForward;
	float3 Left = normalize(cross(Forward, Up));
	if (abs(dot(Forward, Up)) > 0.99f)
	{
		Left = -ViewRight;
	}
	Forward = normalize(cross(Up, Left));
	float3x3 LocalReferencial = transpose(float3x3(Forward, Left, Up));
	return LocalReferencial;
}