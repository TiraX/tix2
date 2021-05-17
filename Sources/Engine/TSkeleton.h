/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TBoneInfo
	{
		int32 ParentIndex;
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
		void AddBone(const TBoneInfo& Bone);
		void ComputeInvBindMatrices();

		void SetBonePos(int32 BoneIndex, const vector3df& InPos);
		void SetBoneRot(int32 BoneIndex, const quaternion& InRot);
		void SetBoneScale(int32 BoneIndex, const vector3df& InScale);

		void BuildGlobalPoses();
		void GatherBoneData(TVector<float>& BoneData, const TVector<uint32>& BoneMap);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		int32 GetBones() const
		{
			return (int32)Bones.size();
		}
	protected:

	public:
		//FUniformBufferPtr SkeletonResource;

	protected:
		TVector<TBoneInfo> Bones;
		TVector<matrix4> InvBindMatrix;
		TVector<matrix4> GlobalPoses;
	};
}
