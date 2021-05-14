/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeLight;
	class TNodeSkeletalMesh : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(SkeletalMesh);

	public:
		virtual ~TNodeSkeletalMesh();
		void LinkMeshAndSkeleton(TStaticMeshPtr InMesh, TSkeletonPtr InSkeleton);
		void SetAnimation(TAnimSequencePtr InAnim);

		virtual void Tick(float Dt) override;

	protected:

	protected:
		TStaticMeshPtr StaticMesh;
		TSkeletonPtr Skeleton;
		TAnimSequencePtr Animation;
	};

} // end namespace tix

