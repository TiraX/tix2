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
		, SkeletalMeshFlag(0)
	{
	}

	TNodeSkeletalMesh::~TNodeSkeletalMesh()
	{
	}

	void TNodeSkeletalMesh::Tick(float Dt)
	{
		TNode::Tick(Dt);

		if (Animation != nullptr)
		{
			TickAnimation(TEngine::GameTime());
		}

		BuildSkeletonMatrix();
	}

	void TNodeSkeletalMesh::UpdateAllTransformation()
	{
		TNode::UpdateAllTransformation();

		// Temp solution, use LocalToWorld matrix, use instance transform in futher for GPU Driven
		if (LinkedPrimitives.size() > 0)
		{
			// Update primtive uniform buffer
			TVector<FPrimitivePtr> Primitives = LinkedPrimitives;
			matrix4 LocalToWorldMat = AbsoluteTransformation;
			ENQUEUE_RENDER_COMMAND(TNodeSkeletalMeshUpdatePrimitiveUniform)(
				[Primitives, LocalToWorldMat]()
				{
					for (FPrimitivePtr P : Primitives)
					{
						P->SetLocalToWorld(LocalToWorldMat);
						P->UpdatePrimitiveBuffer_RenderThread();
					}
				});
		}
	}

	void TNodeSkeletalMesh::SetSceneTileResource(TSceneTileResourcePtr InSceneTileResource)
	{
		SceneTileResourceRef = InSceneTileResource;
	}

	void TNodeSkeletalMesh::LinkMeshAndSkeleton(TStaticMeshPtr InMesh, TSkeletonPtr InSkeleton)
	{
		StaticMesh = InMesh;
		Skeleton = InSkeleton;
		TI_ASSERT(Skeleton->GetBones() <= TSkeleton::MaxBones);

		// Generate primitives and send to render thread
		LinkedPrimitives.clear();
		LinkedPrimitives.reserve(InMesh->GetMeshSectionCount());

		for (uint32 s = 0; s < InMesh->GetMeshSectionCount(); ++s)
		{
			const TMeshSection& MeshSection = InMesh->GetMeshSection(s);
			TI_ASSERT(InMesh->GetMeshBuffer()->MeshBufferResource != nullptr);
			FPrimitivePtr Primitive = ti_new FPrimitive;
			Primitive->SetSkeletalMesh(
				InMesh->GetMeshBuffer()->MeshBufferResource,
				MeshSection.IndexStart,
				MeshSection.Triangles,
				MeshSection.DefaultMaterial
			);
			LinkedPrimitives.push_back(Primitive);
		}
		TVector<FPrimitivePtr> Primitives = LinkedPrimitives;
		if (SceneTileResourceRef != nullptr)
		{
			// Add primitive to scene tile
			FSceneTileResourcePtr RenderThreadSceneTileResource = SceneTileResourceRef->RenderThreadTileResource;
			ENQUEUE_RENDER_COMMAND(AddTNodeSkeletalMeshPrimitivesToFSceneTile)(
				[RenderThreadSceneTileResource, Primitives]()
				{
					const uint32 TotalPrimitives = (uint32)Primitives.size();
					for (uint32 p = 0; p < TotalPrimitives; ++p)
					{
						RenderThreadSceneTileResource->AppendPrimitive(Primitives[p]);
					}
				});
		}
		else
		{
			// Add primitive to somewhere, like FScene??
			TI_ASSERT(0);
		}

		SkeletalMeshFlag |= SKMatrixDirty;
	}

	void TNodeSkeletalMesh::SetAnimation(TAnimSequencePtr InAnim)
	{
		Animation = InAnim;
	}

	void TNodeSkeletalMesh::TickAnimation(float InTime)
	{
		// Calc time and interpolation time
		float AnimTime = InTime * Animation->GetRateScale();
		AnimTime = TMath::FMod(AnimTime, Animation->GetSequenceLength());

		const float FrameLength = Animation->GetFrameLength();

		const int32 Frame0 = (int32)(AnimTime / FrameLength);
		const int32 Frame1 = Frame0 + 1;

		const float t = (AnimTime / FrameLength - FrameLength * Frame0) / FrameLength;

		// Go through tracks
		const TVector<TTrackInfo>& Tracks = Animation->GetTrackInfos();
		const float* FrameData = Animation->GetFrameData().data();

		const int32 NumTracks = (int32)Tracks.size();
		TI_ASSERT(NumTracks == Skeleton->GetBones());

		for (int32 track = 0; track < NumTracks; track++)
		{
			const TTrackInfo Track = Tracks[track];

			const float* TrackKeys = FrameData + Track.KeyDataOffset;

			const vector3df* PosKeys = (vector3df*)TrackKeys;
			const quaternion* RotKeys = (quaternion*)(TrackKeys + Track.NumPosKeys * 3);
			const vector3df* ScaleKeys = (vector3df*)(TrackKeys + Track.NumPosKeys * 3 + Track.NumRotKeys * 4);

			vector3df Pos;
			quaternion Rot;
			vector3df Scale(1, 1, 1);

			if (Track.NumPosKeys > 0)
			{
				if (Frame0 >= Track.NumPosKeys)
				{
					Pos = PosKeys[Track.NumPosKeys - 1];
				}
				else
				{
					Pos = TMath::Lerp(PosKeys[Frame0], PosKeys[Frame1], t);
				}
			}

			if (Track.NumRotKeys > 0)
			{
				if (Frame0 >= Track.NumRotKeys)
				{
					Rot = RotKeys[Track.NumRotKeys - 1];
				}
				else
				{
					Rot.slerp(RotKeys[Frame0], RotKeys[Frame1], t);
				}
			}

			if (Track.NumScaleKeys > 0)
			{
				if (Frame0 >= Track.NumScaleKeys)
				{
					Scale = ScaleKeys[Track.NumScaleKeys - 1];
				}
				else
				{
					Scale = TMath::Lerp(ScaleKeys[Frame0], ScaleKeys[Frame1], t);
				}
			}
			Skeleton->SetBonePos(Track.RefBoneIndex, Pos);
			Skeleton->SetBoneRot(Track.RefBoneIndex, Rot);
			Skeleton->SetBoneScale(Track.RefBoneIndex, Scale);
		}

		// Need to update skeleton matrix
		SkeletalMeshFlag |= SKMatrixDirty;
	}

	void TNodeSkeletalMesh::BuildSkeletonMatrix()
	{
		if ((SkeletalMeshFlag & SKMatrixDirty) != 0)
		{
			// Build matrices from skeleton
			Skeleton->BuildGlobalPoses();
			Skeleton->InitRenderThreadResource();

			// Link skeleton resource to primitives
			TVector<FPrimitivePtr> Primitives = LinkedPrimitives;
			FUniformBufferPtr SkeletonResource = Skeleton->SkeletonResource;
			ENQUEUE_RENDER_COMMAND(PrimitiveSetSkeleton)(
				[Primitives, SkeletonResource]()
				{
					const uint32 TotalPrimitives = (uint32)Primitives.size();
					for (uint32 p = 0; p < TotalPrimitives; ++p)
					{
						Primitives[p]->SetSkeletonResource(SkeletonResource);
					}
				});

			SkeletalMeshFlag &= ~SKMatrixDirty;
		}
	}
}
