#include "S_SkyAtmosphereRS.hlsli"

Texture2D<float4> TransmittanceLut : register(t0);
Texture2D<float4> SkyViewLut : register(t1);
Texture3D<float4> CameraAerialPerspectiveVolumeLut : register(t2);

SamplerState sampler0 : register(s0);

[RootSignature(Sky_RootSig)]
float4 main(PixelShaderInput input) : SV_Target0
{
	float4 sky = TransmittanceLut.Sample(sampler0, input.uv);
	return sky;// float4(input.uv, 0.0, 1.0);
}
