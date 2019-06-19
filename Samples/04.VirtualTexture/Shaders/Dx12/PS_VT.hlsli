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