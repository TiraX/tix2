/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{

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

	class TNodeSceneTile : public TNode, public ILoadingTaskNotifier
	{
		DECLARE_NODE_WITH_CONSTRUCTOR(SceneTile);

	public:
		virtual ~TNodeSceneTile();

		TInstanceBufferPtr GetInstanceBufferByIndex(int32 Index)
		{
			return SceneTileResource->GetInstanceBufferByIndex(Index);
		}

		// interface from ILoadingTaskNotifier
		virtual void NotifyLoadingFinished(TAssetPtr InAsset) override;
	protected:

	protected:
		TSceneTileResourcePtr SceneTileResource;

	};

} // end namespace tix

