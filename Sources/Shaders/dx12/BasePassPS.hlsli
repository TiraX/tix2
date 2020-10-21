struct PSParameters
{
	float4 BaseColorOpacity;	// xyz = BaseColor, w = Opacity;
	float4 MRS_;	// x = Metallic, y = Roughness, z = Specular, w = Reserved;
	float4 EmissiveAO;	// xyz = EmissiveColor, w = AO;
	float4 Normal_;	// xyz = Normal, w = Reserved;
};

PSParameters InitPSParameters()
{
	PSParameters Parameters;
	Parameters.BaseColorOpacity = float4(0, 0, 0, 1);
	Parameters.MRS_ = float4(0, 1, 0.5, 0);
	Parameters.EmissiveAO = float4(0, 0, 0, 1);
	Parameters.Normal_ = float4(0, 0, 1, 0);

	return Parameters;
}

float4 CalculatePS(in PSParameters Parameters)
{
	return Parameters.BaseColorOpacity;
}