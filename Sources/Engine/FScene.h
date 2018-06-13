/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FScene
	{
	public:
		FScene();
		~FScene();
		
		void SetRootNode(FNode * Node);
		void AddNode(FNode * Node, FNode * Parent);
		void RemoveNode(FNode * Node);

	protected:
		FNode * RootNode;
		TVector<FMeshRelevance> StaticDrawList;
	};
} // end namespace tix
