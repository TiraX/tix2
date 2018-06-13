/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FNodeStaticMesh : public FNode
	{
	public:
		FNodeStaticMesh(E_NODE_TYPE Type, FNode* parent);
		virtual ~FNodeStaticMesh();

		void SetMeshBuffer(FMeshBufferPtr InMeshBuffer);
	protected:

	protected:
		FMeshBufferPtr MeshBuffer;
	};
} // end namespace tix

