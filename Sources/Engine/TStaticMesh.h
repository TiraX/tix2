/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "TMeshBuffer.h"

namespace tix
{
	// TStaticMesh, hold static mesh components like mesh buffer, mesh sections, collisions, occluders
	class TI_API TStaticMesh : public TResource
	{
	public:
		TStaticMesh(TMeshBufferPtr InMB);
		virtual ~TStaticMesh();

		virtual void InitRenderThreadResource();
		virtual void DestroyRenderThreadResource();

		TMeshBufferPtr GetMeshBuffer()
		{
			return MeshBuffer;
		}

		TMeshBufferPtr GetOccludeMesh()
		{
			return OccludeMesh;
		}

		void AddMeshSection(const TMeshSection& InSection)
		{
			TI_ASSERT(InSection.Triangles <= MeshBuffer->GetIndicesCount() / 3);
			MeshSections.push_back(InSection);
		}

		void SetCollision(TCollisionSetPtr InCollision)
		{
			CollisionSet = InCollision;
		}

		void SetOccludeMesh(TMeshBufferPtr InOcclude)
		{
			OccludeMesh = InOcclude;
		}
		
		uint32 GetMeshSectionCount() const
		{
			return (uint32)MeshSections.size();
		}

		const TMeshSection& GetMeshSection(int32 SectionIndex) const
		{
			TI_TODO("Remove Mesh Section Info, treat mesh section as another MeshBuffer.");
			return MeshSections[SectionIndex];
		}

		void CreateOccludeMeshFromCollision();

	private:
		TMeshBufferPtr MeshBuffer;
		TVector<TMeshSection> MeshSections;
		TCollisionSetPtr CollisionSet;
		TMeshBufferPtr OccludeMesh;
	};
}
