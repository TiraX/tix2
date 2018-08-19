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

		enum SceneFlag
		{
			ViewProjectionDirty = 1 << 0,
		};

		void SetViewProjection(const FViewProjectionInfo& Info);

		void SetRootNode(FNode * Node);
		void AddNode(FNode * Node, FNode * Parent);
		void RemoveNode(FNode * Node);

		bool HasSceneFlag(SceneFlag Flag) const
		{
			return (SceneFlags & Flag) != 0;
		}

		void SetSceneFlag(SceneFlag Flag, bool Enable)
		{
			if (Enable)
			{
				SceneFlags |= Flag;
			}
			else
			{
				SceneFlags &= ~Flag;
			}
		}

		const FViewProjectionInfo& GetViewProjection() const
		{
			return ViewProjection;
		}

		const TVector<FMeshRelevance>& GetStaticDrawList() const
		{
			return StaticDrawList;
		}
	protected:
		FNode * RootNode;
		TVector<FMeshRelevance> StaticDrawList;

		uint32 SceneFlags;

		FViewProjectionInfo ViewProjection;
	};
} // end namespace tix
