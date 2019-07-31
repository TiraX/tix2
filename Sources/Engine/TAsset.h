/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TAssetLoader
	{
	public:
        TAssetLoader() {}
        virtual ~TAssetLoader() {}
		virtual void Load(TAssetPtr InAsset) = 0;
	};

	class TAssetLoaderSync : public TAssetLoader
	{
	public:
		virtual void Load(TAssetPtr InAsset) override;
	};

	class TAssetLoaderAync : public TAssetLoader
	{
	public:
		virtual void Load(TAssetPtr InAsset) override;
	};

	//////////////////////////////////////////////////////////////////
	class TAsset : public IReferenceCounted
	{
	public:
		TAsset(const TString& InName)
			: AssetName(InName)
			, Loader(nullptr)
			, LoadingNotifier(nullptr)
		{}

		~TAsset()
		{
			SAFE_DELETE(Loader);
			for (auto& Res : Resources)
			{
				Res = nullptr;
			}
		}

		void Load(bool bAync);

		void SetLoadingNotifier(ILoadingTaskNotifier * Notifier)
		{
			LoadingNotifier = Notifier;
		}

		TResource* GetResourcePtr(int32 Index = 0)
		{
			return Resources[Index].get();
		}

		void InitRenderThreadResource()
		{
			for (auto& Res : Resources)
			{
				Res->InitRenderThreadResource();
			}
		}

		void DestroyRenderThreadResource()
		{
			for (auto& Res : Resources)
			{
				Res->DestroyRenderThreadResource();
			}
		}

		void ClearResources()
		{
			for (auto& Res : Resources)
			{
				Res = nullptr;
			}
			Resources.clear();
		}

		bool HasReference() const
		{
			for (auto& Res : Resources)
			{
				if (Res->referenceCount() > 1)
				{
					return true;
				}
			}
			return false;
		}

		const TString& GetName() const
		{
			return AssetName;
		}

		const TVector<TResourcePtr>& GetResources() const
		{
			return Resources;
		}
		
	private:
		TString AssetName;

		// Hold an source file for asynchronous loading
		TAssetLoader * Loader;
		ILoadingTaskNotifier * LoadingNotifier;

		// Resource loaded
		TVector<TResourcePtr> Resources;

		friend class TAssetLoaderSync;
		friend class TAssetLoaderAync;
		friend class TAssetAyncLoadTask;
	};
}
