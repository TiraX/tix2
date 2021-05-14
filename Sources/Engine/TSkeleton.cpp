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
		Bones.reserve(NumBones);
	}

	TSkeleton::~TSkeleton()
	{
	}

	void TSkeleton::AddBone(const TBoneInfo& Bone)
	{
		Bones.push_back(Bone);
	}

	void TSkeleton::AddBone(int32 ParentIndex, const vector3df& Trans, const quaternion& Rot, const vector3df& Scale)
	{
		TBoneInfo Bone;
		Bone.ParentIndex = ParentIndex;
		Bone.Pos = Trans;
		Bone.Rot = Rot;
		Bone.Scale = Scale;

		Bones.push_back(Bone);
	}
}