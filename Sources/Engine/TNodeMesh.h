/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TNodeMesh : public TNode
	{
	public:
		TNodeMesh(TNode* parent, E_NODE_TYPE type = ENT_MESH);
		virtual ~TNodeMesh();

	};

} // end namespace tix

