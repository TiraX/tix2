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

	TResourceObjectPtr TResourceLibrary::LoadResource(const TString& ResFilename)
	{
		if (ResourceObjects.find(ResFilename) == ResourceObjects.end())
		{
			// Load resource to library
			TResourceObjectPtr ResourceObject = ti_new TResourceObject;
			ResourceObject-> SourceFile = ti_new TResourceFile;
			if (ResourceObject->SourceFile->Load(ResFilename))
			{
				ResourceObject->Resource = ResourceObject->SourceFile->CreateResource();
				if (ResourceObject->Resource != nullptr)
				{
					// Release source file
					ResourceObject->SourceFile = nullptr;
					// Init render thread resources
					ResourceObject->Resource->InitRenderThreadResource();
					ResourceObjects[ResFilename] = ResourceObject;
					return ResourceObject;
				}
			}
			// Release source file if create resource failed.
			ResourceObject->SourceFile = nullptr;

			return nullptr;
		}
		else
		{
			return ResourceObjects[ResFilename];
		}
	}

	TResourceObjectPtr TResourceLibrary::LoadResourceAysc(const TString& ResFilename)
	{
		if (ResourceObjects.find(ResFilename) == ResourceObjects.end())
		{
			// Load resource to library
			TResourceObjectPtr ResourceObject = ti_new TResourceObject;
			TResourceLoadingTask * LoadingTask = ti_new TResourceLoadingTask(ResFilename, ResourceObject);
			ResourceObjects[ResFilename] = ResourceObject;
			TThreadLoading::Get()->AddTask(LoadingTask);
			return ResourceObject;
		}
		else
		{
			return ResourceObjects[ResFilename];
		}
	}

	TResourceObjectPtr TResourceLibrary::CreateShaderResource(const TShaderNames& ShaderNames)
	{
		TString ShaderKey = ShaderNames.GetSearchKey();
		if (ResourceObjects.find(ShaderKey) == ResourceObjects.end())
		{
			TI_TODO("Remove this CreateShaderResource() function, merge it into LoadResource");
			// Create a shader resource
			TResourceObjectPtr ResourceObject = ti_new TResourceObject;
			ResourceObject->Resource = ti_new TShader(ShaderNames);
			ResourceObject->Resource->InitRenderThreadResource();
			ResourceObjects[ShaderKey] = ResourceObject;
			return ResourceObject;
		}
		else
		{
			return ResourceObjects[ShaderKey];
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
		MapResourceObjects::iterator it = ResourceObjects.begin();
		for (; it != ResourceObjects.end(); )
		{
			if (it->second->Resource->referenceCount() == 1)
			{
				_LOG(Log, "unused resource : [%s], removed\n", it->first.c_str());
				it->second->Resource->DestroyRenderThreadResource();
				it->second->Resource = nullptr;
				it->second = nullptr;
				ResourceObjects.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}

	void TResourceLibrary::RemoveAllResources()
	{
		MapResourceObjects::iterator it = ResourceObjects.begin();
		for (; it != ResourceObjects.end(); ++ it)
		{
			it->second->Resource->DestroyRenderThreadResource();
			it->second->Resource = nullptr;
			it->second = nullptr;
		}
		ResourceObjects.clear();
	}
}