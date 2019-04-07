/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TResource.h"
#include "TAssetFile.h"

namespace tix
{
	void TAssetLoaderSync::Load(TAssetPtr InAsset)
	{
		TAssetFilePtr AssetFile = ti_new TAssetFile;
		if (AssetFile->Load(InAsset->GetName()))
		{
			AssetFile->CreateResource(InAsset->Resources);
			if (InAsset->Resources.size() > 0)
			{
				// Init render thread resources
				for (auto& Res : InAsset->Resources)
				{
					Res->InitRenderThreadResource();
				}
			}
		}
		AssetFile = nullptr;
	}

	class TAssetAyncLoadTask : public TLoadingTask
	{
	public:
		TAssetAyncLoadTask(TAssetPtr InAsset)
			: Asset(InAsset)
		{}

		virtual void ExecuteInIOThread() override
		{
			// Read file content to FileBuffer
			TI_ASSERT(AssetFile == nullptr);
			AssetFile = ti_new TAssetFile;
			AssetFile->ReadFile(Asset->GetName());
		}

		virtual void ExecuteInLoadingThread() override
		{
			AssetFile->ParseFile();
			AssetFile->CreateResource(Asset->Resources);
			AssetFile = nullptr;
		}

		virtual void ExecuteInMainThread() override
		{
			// Init render thread resource
			for (auto& Res : Asset->Resources)
			{
				Res->InitRenderThreadResource();
			}
		}

	private:
		TAssetPtr Asset;
		TAssetFilePtr AssetFile;
	};

	void TAssetLoaderAync::Load(TAssetPtr InAsset)
	{
		// Add loading task
		TAssetAyncLoadTask * LoadingTask = ti_new TAssetAyncLoadTask(InAsset);
		TThreadLoading::Get()->AddTask(LoadingTask);
	}

	//////////////////////////////////////////////////////////////////

	void TAsset::Load(bool bAync)
	{
		TI_ASSERT(Loader == nullptr);
		if (bAync)
		{
			Loader = ti_new TAssetLoaderAync;
		}
		else
		{
			Loader = ti_new TAssetLoaderSync;
		}
		Loader->Load(this);
	}
}
