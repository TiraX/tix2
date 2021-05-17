#include "Common.hlsli"
#include "S_BasePassRS.hlsli"
#include "VS_SkinMesh.hlsli"

[RootSignature(BasePass_SkeletalMesh_RootSig)]
VSOutput main(VSInput vsInput)
{
    VSOutput vsOutput;

    // Skin
    int BoneIndex = vsInput.blend_index.x;
    float3x4 BoneMatrix = float3x4(BoneData[vsInput.blend_index.x * 3 + 0], BoneData[vsInput.blend_index.x * 3 + 1], BoneData[vsInput.blend_index.x * 3 + 2]) * vsInput.blend_weight.x;
    BoneMatrix += float3x4(BoneData[vsInput.blend_index.y * 3 + 0], BoneData[vsInput.blend_index.y * 3 + 1], BoneData[vsInput.blend_index.y * 3 + 2]) * vsInput.blend_weight.y;
    BoneMatrix += float3x4(BoneData[vsInput.blend_index.z * 3 + 0], BoneData[vsInput.blend_index.z * 3 + 1], BoneData[vsInput.blend_index.z * 3 + 2]) * vsInput.blend_weight.z;
    BoneMatrix += float3x4(BoneData[vsInput.blend_index.w * 3 + 0], BoneData[vsInput.blend_index.w * 3 + 1], BoneData[vsInput.blend_index.w * 3 + 2]) * vsInput.blend_weight.w;

    SkinnedPosition = mul(BoneMatrix, float4(vsInput.position, 1));
    //for (int i = 0; i < 4; ++i)
    //{
    //    if (vsInput.blend_weight[i] > 0.0)
    //    {
    //        float3 Tmp;
    //        float4 Pos1 = float4(vsInput.position, 1.0);
    //        int index = vsInput.blend_index[i] * 3;
    //        Tmp.x = dot(Pos1, BoneData[index + 0]);
    //        Tmp.y = dot(Pos1, BoneData[index + 1]);
    //        Tmp.z = dot(Pos1, BoneData[index + 2]);
    //        SkinnedPosition += vsInput.blend_weight[i] * Tmp;
    //    }
    //}

	float3 WorldPosition = GetWorldPosition(SkinnedPosition);
	vsOutput.position = mul(float4(WorldPosition, 1.0), ViewProjection);
	vsOutput.texcoord0 = GetTextureCoords(vsInput);

	half3x3 RotMat = GetWorldRotationMat(vsInput);
    vsOutput.normal = TransformNormal(vsInput.normal * 2.0 - 1.0, RotMat);
    vsOutput.tangent = TransformNormal(vsInput.tangent * 2.0 - 1.0, RotMat);
	vsOutput.view = ViewPos - vsInput.position;

    return vsOutput;
}
