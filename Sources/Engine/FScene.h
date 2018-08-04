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

		void SetViewProjection(const FViewProjectionInfo& Info);

		void SetRootNode(FNode * Node);
		void AddNode(FNode * Node, FNode * Parent);
		void RemoveNode(FNode * Node);

		const TVector<FMeshRelevance>& GetStaticDrawList() const
		{
			return StaticDrawList;
		}

		// Temp function, remove after refactor
		void CollectAllMeshRelevance();
		void TranverseNode(FNode * Node);


	protected:
		FNode * RootNode;
		TVector<FMeshRelevance> StaticDrawList;

		FViewProjectionInfo ViewProjection;
	};
} // end namespace tix
