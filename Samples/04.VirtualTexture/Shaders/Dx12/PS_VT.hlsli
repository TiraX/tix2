float mip_map_level(in float2 texture_coordinate, float texture_size) // in texel units
{
	float2  dx_vtc = ddx(texture_coordinate * texture_size);
	float2  dy_vtc = ddy(texture_coordinate * texture_size);
	float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));
	return max(0.0, 0.5 * log2(delta_max_sqr));
	//return ddx(texture_coordinate).x;
}

float4 GetVTTextureCoords(in float2 texcoord)
{
	float4 TexCoord;
	TexCoord.xy = frac(texcoord) * VTUVTransform.zw + VTUVTransform.xy;
	TexCoord.zw = VTDebugInfo.xy;
	return TexCoord;
}

#if (VT_ENABLED)
Texture2D<float4> EB_IndirectTexture : register(t0);
Texture2D<float4> EB_PhysicPageAtlas : register(t1);
SamplerState PointSampler : register(s0);
SamplerState LinearSampler : register(s1);

float4 GetBaseColor(in float2 texcoord)
{
	const int VTSize = 16 * 1024;
	const int PPSize = 256;
	const int PhysicAtlasSize = 32;
	const float PAInv = 1.0 / PhysicAtlasSize;

	float2 VTCoord = frac(texcoord) * VTUVTransform.zw + VTUVTransform.xy;

	float4 Indirect = EB_IndirectTexture.Sample(PointSampler, VTCoord);
	// coord in virtual texture - 
	float2 VTPos = VTCoord * VTSize / PPSize;
	float2 Coord = frac(VTPos);
	float2 PPCoord = (Indirect.xy * 256 + Coord) * PAInv;

	float4 Color = EB_PhysicPageAtlas.Sample(LinearSampler, PPCoord);
	return Color;
}
#else	// VT_ENABLED

Texture2D<float4> TexBaseColor : register(t0);
SamplerState LinearSampler : register(s0);

float4 GetBaseColor(in float2 texcoord)
{
	return TexBaseColor.Sample(LinearSampler, texcoord);
}
#endif