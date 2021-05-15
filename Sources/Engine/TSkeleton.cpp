/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TSkeleton.h"

namespace tix
{
	TSkeleton::TSkeleton()
		: TResource(ERES_SKELETON)
	{
	}

	TSkeleton::TSkeleton(int32 NumBones)
		: TResource(ERES_SKELETON)
	{
		BoneParents.reserve(NumBones);
		InvBindMatrix.reserve(NumBones);

		BoneTransforms.resize(NumBones);
	}

	TSkeleton::~TSkeleton()
	{
	}

	void TSkeleton::AddBone(int32 ParentIndex, const vector3df& InvTrans, const quaternion& InvRot, const vector3df& InvScale)
	{
		TBoneInitInfo Bone;
		Bone.ParentIndex = ParentIndex;
		Bone.InvPos = InvTrans;
		Bone.InvRot = InvRot;
		Bone.InvScale = InvScale;

		AddBone(Bone);
	}

	void TSkeleton::AddBone(const TBoneInitInfo& Bone)
	{
		BoneParents.push_back(Bone.ParentIndex);

		matrix4 InvMat;
		Bone.InvRot.getMatrix(InvMat);
		InvMat.postScale(Bone.InvScale);
		InvMat.setTranslation(Bone.InvPos);
		InvBindMatrix.push_back(InvMat);
	}

	void TSkeleton::SetBonePos(int32 BoneIndex, const vector3df& InPos)
	{
		BoneTransforms[BoneIndex].Pos = InPos;
	}

	void TSkeleton::SetBoneRot(int32 BoneIndex, const quaternion& InRot)
	{
		BoneTransforms[BoneIndex].Rot = InRot;
	}

	void TSkeleton::SetBoneScale(int32 BoneIndex, const vector3df& InScale)
	{
		BoneTransforms[BoneIndex].Scale = InScale;
	}

	inline void MakeMatrix(matrix4& Mat, const vector3df& Pos, const quaternion& Rot, const vector3df& Scale)
	{
		Rot.getMatrix(Mat);
		Mat.postScale(Scale);
		Mat.setTranslation(Pos);
	}

	void TSkeleton::InitRenderThreadResource()
	{
		//TI_ASSERT(SkeletonResource == nullptr);
		// Skeleton Bone info resource always need to re-create
		SkeletonResource = FRHI::Get()->CreateUniformBuffer(sizeof(float)*12*MaxBones, 1, 0);

		FUniformBufferPtr SkeletonDataResource = SkeletonResource;
		TVector<float> BoneData = BoneMatricsData;
		ENQUEUE_RENDER_COMMAND(TSkeletonUpdateSkeletonResource)(
			[SkeletonDataResource, BoneData]()
			{
				FRHI::Get()->UpdateHardwareResourceUB(SkeletonDataResource, BoneData.data());
			});
	}

	void TSkeleton::DestroyRenderThreadResource()
	{
		TI_ASSERT(SkeletonResource != nullptr);
		
		FUniformBufferPtr SkeletonDataResource = SkeletonResource;
		ENQUEUE_RENDER_COMMAND(TSkeletonDestroySkeletonResource)(
			[SkeletonDataResource]()
			{
				//SkeletonDataResource = nullptr;
			});
		SkeletonDataResource = nullptr;
		SkeletonResource = nullptr;
	}

	void TSkeleton::BuildGlobalPoses()
	{
		TVector<matrix4> GlobalPoses;
		GlobalPoses.resize(BoneParents.size());

		// Update Tree
		const int32 NumBones = (int32)BoneParents.size();
		for (int32 b = 0; b < NumBones; b++)
		{
			const TBoneTransform& BTrans = BoneTransforms[b];

			if (BoneParents[b] < 0)
			{
				// Root
				MakeMatrix(GlobalPoses[b], BTrans.Pos, BTrans.Rot, BTrans.Scale);
			}
			else
			{
				matrix4 Mat;
				MakeMatrix(Mat, BTrans.Pos, BTrans.Rot, BTrans.Scale);

				GlobalPoses[b] = GlobalPoses[BoneParents[b]] * Mat;
			}
		}

		// Multi with InvBindPose
		for (int32 b = 0; b < NumBones; b++)
		{
			GlobalPoses[b] = InvBindMatrix[b] * GlobalPoses[b];
		}

		// Gather data to 4x3 matrices

		BoneMatricsData.resize(MaxBones * 4 * 3);
		for (int i = 0; i < NumBones; ++i)
		{
			const matrix4 Mat = GlobalPoses[i];
			BoneMatricsData[i * 4 * 3 + 0] = Mat[0];
			BoneMatricsData[i * 4 * 3 + 1] = Mat[4];
			BoneMatricsData[i * 4 * 3 + 2] = Mat[8];
			BoneMatricsData[i * 4 * 3 + 3] = Mat[12];

			BoneMatricsData[i * 4 * 3 + 4] = Mat[1];
			BoneMatricsData[i * 4 * 3 + 5] = Mat[5];
			BoneMatricsData[i * 4 * 3 + 6] = Mat[9];
			BoneMatricsData[i * 4 * 3 + 7] = Mat[13];

			BoneMatricsData[i * 4 * 3 + 8] = Mat[2];
			BoneMatricsData[i * 4 * 3 + 9] = Mat[6];
			BoneMatricsData[i * 4 * 3 + 10] = Mat[10];
			BoneMatricsData[i * 4 * 3 + 11] = Mat[14];
		}
	}
}