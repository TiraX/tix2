
float3x3 GetWorldRotationMat(in VFInput Input)
{
	return float3x3(Input.ins_transform0.xyz, Input.ins_transform1.xyz, Input.ins_transform2.xyz);
}

float3 GetWorldPosition(in VFInput Input)
{
	float3x3 RotMat = float3x3(Input.ins_transform0.xyz, Input.ins_transform1.xyz, Input.ins_transform2.xyz);
	float3 position = mul(Input.position, RotMat);
	position += Input.ins_transition.xyz;

	return position;
}

half3 TransformNormal(in half3 Normal, in half3x3 RotMat)
{
	return mul(Normal, RotMat);
}

VStoPS CalculateVS(in VFInput Input)
{
	VStoPS Interporlation;

	float3 WorldPosition = GetWorldPosition(Input);
	Interporlation.position = mul(float4(WorldPosition, 1.0), ViewProjection);
	Interporlation.texcoord0 = Input.texcoord0;

	half3x3 RotMat = GetWorldRotationMat(Input);
	Interporlation.normal = TransformNormal(Input.normal * 2.0 - 1.0, RotMat);
	Interporlation.tangent = TransformNormal(Input.tangent * 2.0 - 1.0, RotMat);
	Interporlation.view = ViewPos - Input.position;

	return Interporlation;
}

VStoPS CalculateVS(in VFInput Input, in float3 WorldPositionOffset)
{
	VStoPS Interporlation;

	float3 WorldPosition = GetWorldPosition(Input) + WorldPositionOffset;
	Interporlation.position = mul(float4(WorldPosition, 1.0), ViewProjection);
	Interporlation.texcoord0 = Input.texcoord0;

	half3x3 RotMat = GetWorldRotationMat(Input);
	Interporlation.normal = TransformNormal(Input.normal * 2.0 - 1.0, RotMat);
	Interporlation.tangent = TransformNormal(Input.tangent * 2.0 - 1.0, RotMat);
	Interporlation.view = ViewPos - Input.position;

	return Interporlation;
}