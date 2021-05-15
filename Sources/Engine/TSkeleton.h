/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TBoneInitInfo
	{
		int32 ParentIndex;
		vector3df InvPos;
		quaternion InvRot;
		vector3df InvScale;
	};

	struct TBoneTransform
	{
		vector3df Pos;
		quaternion Rot;
		vector3df Scale;
	};

	// TSkeleton, hold skeleton info for skeletal mesh
	class TI_API TSkeleton : public TResource
	{
	public:
		static const int32 MaxBones = 128;

		TSkeleton();
		TSkeleton(int32 NumBones);
		~TSkeleton();

		void AddBone(int32 ParentIndex, const vector3df& InvTrans, const quaternion& InvRot, const vector3df& InvScale);
		void AddBone(const TBoneInitInfo& Bone);

		void SetBonePos(int32 BoneIndex, const vector3df& InPos);
		void SetBoneRot(int32 BoneIndex, const quaternion& InRot);
		void SetBoneScale(int32 BoneIndex, const vector3df& InScale);

		void BuildGlobalPoses();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		int32 GetBones() const
		{
			return (int32)BoneParents.size();
		}
	protected:

	public:
		FUniformBufferPtr SkeletonResource;

	protected:
		TVector<int32> BoneParents;
		TVector<TBoneTransform> BoneTransforms;
		TVector<matrix4> InvBindMatrix;

		TVector<float> BoneMatricsData;
	};
}
