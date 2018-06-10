/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeMesh.h"

namespace tix
{
	TNodeMesh::TNodeMesh(TNode* parent, E_NODE_TYPE type)
		: TNode(type, parent)
	{
	}

	TNodeMesh::~TNodeMesh()
	{
	}
}
