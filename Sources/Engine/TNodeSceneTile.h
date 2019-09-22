/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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

		TInstanceBufferPtr GetInstanceBufferByIndex(int32 Index)
		{
			return SceneTileResource->GetInstanceBufferByIndex(Index);
		}
	protected:

	protected:
		TSceneTileResourcePtr SceneTileResource;

		friend class TSceneTileLoadingFinishDelegate;
	};

} // end namespace tix

