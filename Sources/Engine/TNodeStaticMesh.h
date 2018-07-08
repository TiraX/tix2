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
		
		virtual void AddMeshBuffer(TMeshBufferPtr MeshBuffer) override;
		virtual void CreateRenderThreadNode() override;

	protected:
		TMeshBufferPtr MeshBuffer;
	};

} // end namespace tix

