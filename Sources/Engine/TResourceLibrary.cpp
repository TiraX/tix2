/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TResourceLibrary.h"
#include "TThreadLoading.h"

namespace tix
{
	TResourceLibrary* TResourceLibrary::s_instance = nullptr;

	TResourceLibrary* TResourceLibrary::Get()
	{
		return s_instance;
	}

	TResourceLibrary::TResourceLibrary()
	{
		TI_ASSERT(s_instance == nullptr);
		s_instance = this;
	}

	TResourceLibrary::~TResourceLibrary()
	{
		RemoveAllResources();
		TI_ASSERT(s_instance != nullptr);
		s_instance = nullptr;
	}

	TResourceTaskPtr TResourceLibrary::LoadResource(const TString& ResFilename)
	{
		if (ResourceTasks.find(ResFilename) == ResourceTasks.end())
		{
			// Load resource to library
			TResourceTaskPtr ResourceObject = ti_new TResourceTask;
			ResourceObject-> SourceFile = ti_new TResourceFile;
			if (ResourceObject->SourceFile->Load(ResFilename))
			{
				 ResourceObject->SourceFile->CreateResource(ResourceObject->Resources);
				if (ResourceObject->Resources.size() > 0)
				{
					// Release source file
					ResourceObject->SourceFile = nullptr;
					// Init render thread resources
					for (auto& Res : ResourceObject->Resources)
					{
						Res->InitRenderThreadResource();
					}
					ResourceTasks[ResFilename] = ResourceObject;
					return ResourceObject;
				}
			}
			// Release source file if create resource failed.
			ResourceObject->SourceFile = nullptr;

			return nullptr;
		}
		else
		{
			return ResourceTasks[ResFilename];
		}
	}

	TResourceTaskPtr TResourceLibrary::LoadResourceAysc(const TString& ResFilename)
	{
		if (ResourceTasks.find(ResFilename) == ResourceTasks.end())
		{
			// Load resource to library
			TResourceTaskPtr ResourceTask = ti_new TResourceTask;
			TResourceLoadingTask * LoadingTask = ti_new TResourceLoadingTask(ResFilename, ResourceTask);
			ResourceTasks[ResFilename] = ResourceTask;
			TThreadLoading::Get()->AddTask(LoadingTask);
			return ResourceTask;
		}
		else
		{
			return ResourceTasks[ResFilename];
		}
	}

	void TResourceLibrary::LoadScene(const TString& ResFilename)
	{
		// Load resource to library
		TResourceFilePtr ResFile = ti_new TResourceFile;
		if (ResFile->Load(ResFilename))
		{
			ResFile->LoadScene();
		}
	}

	void TResourceLibrary::RemoveUnusedResources()
	{
		MapResourceTasks::iterator it = ResourceTasks.begin();
		for (; it != ResourceTasks.end(); )
		{
			if (!it->second->HasReference())
			{
				_LOG(Log, "unused resource : [%s], removed\n", it->first.c_str());
				it->second->DestroyRenderThreadResource();
				it->second->ClearResources();
				it->second = nullptr;
				ResourceTasks.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}

	void TResourceLibrary::RemoveAllResources()
	{
		MapResourceTasks::iterator it = ResourceTasks.begin();
		for (; it != ResourceTasks.end(); ++ it)
		{
			it->second->DestroyRenderThreadResource();
			it->second->ClearResources();
			it->second = nullptr;
		}
		ResourceTasks.clear();
	}
}