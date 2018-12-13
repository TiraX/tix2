#include "S_SSSBlur.hlsli"

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
};

cbuffer SSSBlurParam : register(b0)
{
	float4 BlurDir;
	float4 BlurParam;	// x = sssWidth; y = sssFov; z = maxOffsetMm
};

Texture2D TexColor : register(t0);
Texture2D TexDepth : register(t1);
Texture2D TexStrength : register(t2);

SamplerState sampler0 : register(s0);

#define SSSS_N_SAMPLES 17
float4 kernel[] = {
	float4(0.536343, 0.624624, 0.748867, 0),
	float4(0.00317394, 0.000134823, 3.77269e-005, -2),
	float4(0.0100386, 0.000914679, 0.000275702, -1.53125),
	float4(0.0144609, 0.00317269, 0.00106399, -1.125),
	float4(0.0216301, 0.00794618, 0.00376991, -0.78125),
	float4(0.0347317, 0.0151085, 0.00871983, -0.5),
	float4(0.0571056, 0.0287432, 0.0172844, -0.28125),
	float4(0.0582416, 0.0659959, 0.0411329, -0.125),
	float4(0.0324462, 0.0656718, 0.0532821, -0.03125),
	float4(0.0324462, 0.0656718, 0.0532821, 0.03125),
	float4(0.0582416, 0.0659959, 0.0411329, 0.125),
	float4(0.0571056, 0.0287432, 0.0172844, 0.28125),
	float4(0.0347317, 0.0151085, 0.00871983, 0.5),
	float4(0.0216301, 0.00794618, 0.00376991, 0.78125),
	float4(0.0144609, 0.00317269, 0.00106399, 1.125),
	float4(0.0100386, 0.000914679, 0.000275702, 1.53125),
	float4(0.00317394, 0.000134823, 3.77269e-005, 2),
};

// A pass-through function for the (interpolated) color data.
//[RootSignature(SSSBlur_RootSig)]
float4 main(PixelShaderInput input) : SV_TARGET
{
	// Fetch color of current pixel:
	float4 colorM = TexColor.Sample(sampler0, input.uv);
	
	// Fetch linear depth of current pixel:
	float depthM = TexDepth.Sample(sampler0, input.uv).r;

	float sssWidth = BlurParam.x;
	float sssFov = BlurParam.y;
	float maxOffsetMm = BlurParam.z;

	// Calculate the sssWidth scale (1.0 for a unit plane sitting on the
	// projection window):
	float distanceToProjectionWindow = 1.0 / tan(0.5 * sssFov);
	float scale = distanceToProjectionWindow / depthM;

	float2 dir = BlurDir.xy;

	// Calculate the final step to fetch the surrounding pixels:
	float2 finalStep = scale * dir;
	finalStep *= TexStrength.Sample(sampler0, input.uv).r; // Modulate it using the alpha channel.
	finalStep *= 1.0 / (2.0 * sssWidth); // sssWidth in mm / world space unit, divided by 2 as uv coords are from [0 1]

										 // Accumulate the center sample:
	float4 colorBlurred = colorM;
	colorBlurred.rgb *= kernel[0].rgb;

	// Accumulate the other samples:
	[unroll]
		for (int i = 1; i < SSSS_N_SAMPLES; i++) {
			// Fetch color and depth for current sample:
			float2 offset = input.uv + kernel[i].a * finalStep;
			float4 color = TexColor.Sample(sampler0, offset);

//#if SSSS_FOLLOW_SURFACE == 1
			// If the difference in depth is huge, we lerp color back to "colorM":
			float depth = TexDepth.Sample(sampler0, offset).r;

			float s = saturate(abs(depthM - depth) / (distanceToProjectionWindow * (maxOffsetMm / sssWidth)));
			s = min(1, s*1.5); // custom / user definable scaling

			color.rgb = lerp(color.rgb, colorM.rgb, s);
//#endif

			// Accumulate:
			colorBlurred.rgb += kernel[i].rgb * color.rgb;
		}

	return colorBlurred;
}
