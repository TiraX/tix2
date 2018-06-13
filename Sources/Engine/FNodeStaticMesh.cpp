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

	void FNodeStaticMesh::SetMeshBuffer(FMeshBufferPtr InMeshBuffer)
	{
		MeshBuffer = InMeshBuffer;
	}
}
