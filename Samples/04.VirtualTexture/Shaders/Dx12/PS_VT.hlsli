float mip_map_level(in float2 texture_coordinate) // in texel units
{
	float2  dx_vtc = ddx(texture_coordinate);
	float2  dy_vtc = ddy(texture_coordinate);
	float delta_max_sqr = max(dot(dx_vtc, dx_vtc), dot(dy_vtc, dy_vtc));
	return 0.5 * log2(delta_max_sqr);
}
