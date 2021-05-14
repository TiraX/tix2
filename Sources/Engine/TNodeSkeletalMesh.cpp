/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeSkeletalMesh.h"

namespace tix
{
	TNodeSkeletalMesh::TNodeSkeletalMesh(TNode* parent)
		: TNode(TNodeSkeletalMesh::NODE_TYPE, parent)
	{
	}

	TNodeSkeletalMesh::~TNodeSkeletalMesh()
	{
	}

	void TNodeSkeletalMesh::Tick(float Dt)
	{
		TNode::Tick(Dt);
	}

	void TNodeSkeletalMesh::LinkMeshAndSkeleton(TStaticMeshPtr InMesh, TSkeletonPtr InSkeleton)
	{
		StaticMesh = InMesh;
		Skeleton = InSkeleton;
	}

	void TNodeSkeletalMesh::SetAnimation(TAnimSequencePtr InAnim)
	{
		Animation = InAnim;
	}
}
