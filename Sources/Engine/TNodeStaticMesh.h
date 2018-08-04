/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeStaticMesh : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(StaticMesh);

	public:
		virtual ~TNodeStaticMesh();

		struct TMeshDrawRelevance
		{
			TMeshBufferPtr MeshBuffer;
			TPipelinePtr Pipeline;

			// TODO: place holder REFACTOR
			int32 Material;
			int32 CastShadow;
			int32 ReceiveShadow;
		};

		virtual void AddMeshToDraw(TMeshBufferPtr InMesh, TPipelinePtr InPipeline, int32 InMaterial, int32 InCastShadow, int32 InReceiveShadow);
		virtual void CreateRenderThreadNode() override;

	protected:
		TMeshDrawRelevance DrawRelevance;
	};

} // end namespace tix

