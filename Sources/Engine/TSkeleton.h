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
		TSkeleton();
		TSkeleton(int32 NumBones);
		~TSkeleton();

		void AddBone(const TBoneInfo& Bone);
		void AddBone(int32 ParentIndex, const vector3df& Trans, const quaternion& Rot, const vector3df& Scale);

		virtual void InitRenderThreadResource() override {};
		virtual void DestroyRenderThreadResource() override {};

	protected:

	protected:
		TVector<TBoneInfo> Bones;
	};
}
