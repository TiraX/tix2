/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMeshBuffer.h"
#include "TStaticMesh.h"

namespace tix
{
	TStaticMesh::TStaticMesh(TMeshBufferPtr InMB)
		: TResource(ERES_STATIC_MESH)
		, MeshBuffer(InMB)
	{
	}

	TStaticMesh::~TStaticMesh()
	{
	}

	void TStaticMesh::InitRenderThreadResource()
	{
		MeshBuffer->InitRenderThreadResource();
		if (OccludeMesh != nullptr)
		{
			OccludeMesh->InitRenderThreadResource();
		}
	}

	void TStaticMesh::DestroyRenderThreadResource()
	{
		MeshBuffer->DestroyRenderThreadResource();
		if (OccludeMesh != nullptr)
		{
			OccludeMesh->DestroyRenderThreadResource();
		}
	}

	void TStaticMesh::CreateOccludeMeshFromCollision()
	{
		TI_ASSERT(OccludeMesh == nullptr);
		OccludeMesh = CollisionSet->ConvertToMesh();
		if (OccludeMesh != nullptr)
		{
			OccludeMesh->SetResourceName(MeshBuffer->GetResourceName() + "Occluder");
		}
	}
}