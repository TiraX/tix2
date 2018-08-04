/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TNodeStaticMesh.h"

namespace tix
{
	TNodeStaticMesh::TNodeStaticMesh(TNode* parent)
		: TNode(TNodeStaticMesh::NODE_TYPE, parent)
	{
	}

	TNodeStaticMesh::~TNodeStaticMesh()
	{
	}

	void TNodeStaticMesh::CreateRenderThreadNode()
	{
		CREATE_RENDER_THREAD_NODE(FNodeStaticMesh);
	}

	void TNodeStaticMesh::AddMeshToDraw(TMeshBufferPtr InMesh, TPipelinePtr InPipeline, int32 InMaterial, int32 InCastShadow, int32 InReceiveShadow)
	{
		TI_TODO("Support multi mesh buffer in one node.");
		DrawRelevance.MeshBuffer = InMesh;
		DrawRelevance.Pipeline = InPipeline;
		DrawRelevance.Material = InMaterial;
		DrawRelevance.CastShadow = InCastShadow;
		DrawRelevance.ReceiveShadow = InReceiveShadow;

		// send to render thread node to get mesh buffer render resource
		TI_ASSERT(Node_RenderThread);

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(AddMeshToDrawRenderThread,
			FNode *, Node_RT, Node_RenderThread,
			TNodeStaticMesh::TMeshDrawRelevance, Relevance, DrawRelevance,
			{
				FNodeStaticMesh * NodeMesh = static_cast<FNodeStaticMesh*>(Node_RT);
				NodeMesh->AddMeshToDraw(Relevance.MeshBuffer->MeshBufferResource, Relevance.Pipeline->PipelineResource);
			});
	}
}
