struct VSOutput
{
	float4 position : SV_Position;
	float4 uv : TexCoord0;
	float3 normal : Normal;
	float3 tangent : Tangent;
	float3 view : TexCoord1;
};