/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeStaticMesh.h"

namespace tix
{
	TNodeStaticMesh::TNodeStaticMesh(TNode* parent)
		: TNode(ENT_MESH, parent)
	{
	}

	TNodeStaticMesh::~TNodeStaticMesh()
	{
	}

	void TNodeStaticMesh::CreateRenderThreadNode()
	{
		CREATE_RENDER_THREAD_NODE(FNodeStaticMesh);
	}

	void TNodeStaticMesh::SetMeshBuffer(TMeshBufferPtr InMeshBuffer)
	{
		MeshBuffer = InMeshBuffer;

		// send to render thread node to get mesh buffer render resource
		TI_ASSERT(Node_RenderThread);

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(SetMeshBufferRenderThread,
			FNode *, Node_RT, Node_RenderThread,
			TMeshBufferPtr, MeshBuffer, InMeshBuffer,
			{
				FNodeStaticMesh * NodeMesh = static_cast<FNodeStaticMesh*>(Node_RT);
				FMeshBufferPtr MeshBuffer_RenderResource = RHI->CreateMeshBuffer(MeshBuffer);
				NodeMesh->SetMeshBuffer(MeshBuffer_RenderResource);
			});
	}
}
