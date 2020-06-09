/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TSceneTileLoadingFinishDelegate : public ILoadingFinishDelegate
	{
	public:
		TSceneTileLoadingFinishDelegate(const TString& InLevelName, const TString& InTileName);
		virtual ~TSceneTileLoadingFinishDelegate();
		virtual void LoadingFinished(TAssetPtr InAsset) override;

	private:
		TString LevelName;
		TString TileName;
	};

	//////////////////////////////////////////////////////////////////////////

	class TNodeLevel : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(Level);

	public:
		virtual ~TNodeLevel();

		const TString& GetLevelName() const
		{
			return GetId();
		}

	private:
	};

	//////////////////////////////////////////////////////////////////////////

	class TNodeSceneTile : public TNode
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(SceneTile);

	public:
		virtual ~TNodeSceneTile();

		virtual void UpdateAllTransformation() override;

		TInstanceBufferPtr GetInstanceBuffer()
		{
			return SceneTileResource->GetInstanceBuffer();
		}

		const vector2di& GetTilePosition() const
		{
			return SceneTileResource->Position;
		}
	protected:

	protected:
		TSceneTileResourcePtr SceneTileResource;
		TVector<TAssetPtr> LoadedMeshAssets;

		friend class TSceneTileLoadingFinishDelegate;
	};

} // end namespace tix

