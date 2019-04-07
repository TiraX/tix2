/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TAssetLibrary.h"
#include "TThreadLoading.h"

namespace tix
{
	TAssetLibrary* TAssetLibrary::s_instance = nullptr;

	TAssetLibrary* TAssetLibrary::Get()
	{
		return s_instance;
	}

	TAssetLibrary::TAssetLibrary()
	{
		TI_ASSERT(s_instance == nullptr);
		s_instance = this;
	}

	TAssetLibrary::~TAssetLibrary()
	{
		RemoveAllResources();
		TI_ASSERT(s_instance != nullptr);
		s_instance = nullptr;
	}

	TAssetPtr TAssetLibrary::LoadAsset(const TString& AssetFilename)
	{
		if (AssetsLoaded.find(AssetFilename) == AssetsLoaded.end())
		{
			// Load resource to library
			TAssetPtr Asset = ti_new TAsset(AssetFilename);
			AssetsLoaded[AssetFilename] = Asset;
			Asset->Load(false);
			return Asset;
		}
		else
		{
			return AssetsLoaded[AssetFilename];
		}
	}

	TAssetPtr TAssetLibrary::LoadAssetAysc(const TString& AssetFilename)
	{
		if (AssetsLoaded.find(AssetFilename) == AssetsLoaded.end())
		{
			// Load resource to library
			TAssetPtr Asset = ti_new TAsset(AssetFilename);
			AssetsLoaded[AssetFilename] = Asset;
			Asset->Load(true);
			return Asset;
		}
		else
		{
			return AssetsLoaded[AssetFilename];
		}
	}

	void TAssetLibrary::LoadScene(const TString& AssetFilename)
	{
		TI_ASSERT(0);
		// Load resource to library
		//TAssetFilePtr AssetFile = ti_new TAssetFile;
		//if (ResFile->Load(AssetFilename))
		//{
		//	ResFile->LoadScene();
		//}
	}

	void TAssetLibrary::RemoveUnusedResources()
	{
		MapAssets::iterator it = AssetsLoaded.begin();
		for (; it != AssetsLoaded.end(); )
		{
			if (!it->second->HasReference())
			{
				_LOG(Log, "unused resource : [%s], removed\n", it->first.c_str());
				it->second->DestroyRenderThreadResource();
				it->second->ClearResources();
				it->second = nullptr;
				AssetsLoaded.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}

	void TAssetLibrary::RemoveAllResources()
	{
		MapAssets::iterator it = AssetsLoaded.begin();
		for (; it != AssetsLoaded.end(); ++ it)
		{
			it->second->DestroyRenderThreadResource();
			it->second->ClearResources();
			it->second = nullptr;
		}
		AssetsLoaded.clear();
	}
}