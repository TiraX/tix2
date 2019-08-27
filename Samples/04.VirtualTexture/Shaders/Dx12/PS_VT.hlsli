
static const int VTSize = 16 * 1024;
static const int PPSize = 256;
static const int PhysicAtlasSize = 32;
static const float PAInv = 1.0 / PhysicAtlasSize;

float mip_map_level(in float2 texture_coordinate, float texture_size) // in texel units
{
	float2  dx_vtc = ddx(texture_coordinate * texture_size);
	float2  dy_vtc = ddy(texture_coordinate * texture_size);
	float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));
	return min(6.0, max(0.0, 0.5 * log2(delta_max_sqr)));
}

float4 GetVTTextureCoords(in float2 texcoord)
{
	float4 TexCoord;
	TexCoord.xy = frac(texcoord) * VTUVTransform.zw + VTUVTransform.xy;
	TexCoord.z = mip_map_level(TexCoord.xy, VTSize);
	TexCoord.w = 1.0;
	return TexCoord;
}

#if (VT_ENABLED)
Texture2D<float4> EB_VTArgsIndirect : register(t0);
Texture2D<float4> EB_VTArgsPhysic : register(t1);
SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);

float4 GetBaseColor(in float2 texcoord)
{
	float2 VTCoord = frac(texcoord) * VTUVTransform.zw + VTUVTransform.xy;
	float MipLevel = floor(mip_map_level(VTCoord, VTSize));

	float4 Indirect = EB_VTArgsIndirect.SampleLevel(PointSampler, VTCoord, MipLevel);
	// coord in virtual texture - 
	int VTMipSize = VTSize / pow(2, int(MipLevel));
	float2 VTPos = VTCoord * VTMipSize / PPSize;
	float2 Coord = frac(VTPos) *256 / 258 + 1.0 / 258;
	//float2 Coord = frac(VTPos);
	float2 PPCoord = (floor(Indirect.xy * 256) + Coord) * PAInv;

	float4 Color = EB_VTArgsPhysic.Sample(LinearSampler, PPCoord);
	return Color;
	//return PPCoord.xyxy;
}
#else	// VT_ENABLED
cbuffer MaterialInstanceParameter : register(b1)
{
	float4 CustomScalar0;
};
Texture2D<float4> TexBaseColor : register(t0);
SamplerState LinearSampler : register(s0);

float4 GetBaseColor(in float2 texcoord)
{
	return TexBaseColor.Sample(LinearSampler, texcoord) * (1.0 + CustomScalar0.x * 0.1);
}
#endif