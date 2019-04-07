/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TAssetLibrary
	{
	public:
		TI_API static TAssetLibrary* Get();

		TI_API TAssetPtr LoadAsset(const TString& AssetFilename);
		TI_API TAssetPtr LoadAssetAysc(const TString& AssetFilename, ILoadingTaskNotifier * Notifier = nullptr);

		TI_API void RemoveUnusedResources();

	private:
		static TAssetLibrary* s_instance;
		TAssetLibrary();
		virtual ~TAssetLibrary();

		void RemoveAllResources();
		void AddAsset(const TString& InAssetName, TAssetPtr InAsset);

	private:
		// Assets loaded, can be modified in different threads, make sure it is thread safe.
		typedef TMap< TString, TAssetPtr > MapAssets;
		MapAssets AssetsLoaded;
		TMutex AssetsLock;

		friend class TEngine;
	};
}