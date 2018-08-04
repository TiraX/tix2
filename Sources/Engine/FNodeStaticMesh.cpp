/*
	Copyright (C) 2012
	By Zhao Shuai tirax.cn@gmail.com 2012.7.18
*/

#include "stdafx.h"
#include "FNodeStaticMesh.h"

namespace tix
{
	FNodeStaticMesh::FNodeStaticMesh(E_NODE_TYPE Type, FNode* parent)
		: FNode(Type, parent)
	{
	}

	FNodeStaticMesh::~FNodeStaticMesh()
	{
	}

	void FNodeStaticMesh::AddMeshToDraw(FMeshBufferPtr InMeshBuffer, FPipelinePtr InPipeline)
	{
		MeshBuffer = InMeshBuffer;
		Pipeline = InPipeline;
	}

	void FNodeStaticMesh::AddToStaticMeshList(TVector<FMeshRelevance>& List)
	{
		TI_TODO("Temp fucntion, remove after refactor");
		FMeshRelevance Relevance;
		Relevance.MeshBuffer = MeshBuffer;
		Relevance.Pipeline = Pipeline;
		List.push_back(Relevance);
	}
}
