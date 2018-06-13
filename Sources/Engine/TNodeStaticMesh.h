/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeStaticMesh : public TNode
	{
	public:
		TNodeStaticMesh(TNode* parent);
		virtual ~TNodeStaticMesh();
		
		virtual void SetMeshBuffer(TMeshBufferPtr MeshBuffer) override;
		virtual void CreateRenderThreadNode() override;

	protected:
		TMeshBufferPtr MeshBuffer;
	};

} // end namespace tix

