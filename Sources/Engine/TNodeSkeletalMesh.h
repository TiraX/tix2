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
		void SetSceneTileResource(TSceneTileResourcePtr InSceneTileResource);
		void LinkMeshAndSkeleton(TStaticMeshPtr InMesh, TSkeletonPtr InSkeleton);
		void SetAnimation(TAnimSequencePtr InAnim);

		virtual void Tick(float Dt) override;
		virtual void UpdateAllTransformation() override;

	protected:
		void TickAnimation(float InTime);
		void BuildSkeletonMatrix();

	protected:
		TSceneTileResourcePtr SceneTileResourceRef;
		TStaticMeshPtr StaticMesh;
		TSkeletonPtr Skeleton;
		TAnimSequencePtr Animation;

		TVector<FPrimitivePtr> LinkedPrimitives;
		enum {
			SKMatrixDirty = 1 << 0,
		};
		uint32 SkeletalMeshFlag;
	};

} // end namespace tix

