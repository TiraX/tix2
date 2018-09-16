/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TResourceLibrary.h"

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

	TResourcePtr TResourceLibrary::LoadResource(const TString& ResFilename)
	{
		if (Resources.find(ResFilename) == Resources.end())
		{
			// Load resource to library
			TResFilePtr ResFile = ti_new TResFile;
			if (ResFile->Load(ResFilename))
			{
				TResourcePtr Res = ResFile->CreateResource();
				if (Res != nullptr)
				{
					Res->InitRenderThreadResource();
					Resources[ResFilename] = Res;
					return Res;
				}
			}

			return nullptr;
		}
		else
		{
			return Resources[ResFilename];
		}
	}

	void TResourceLibrary::RemoveUnusedResources()
	{
		MapResources::iterator it = Resources.begin();
		for (; it != Resources.end(); )
		{
			if (it->second->referenceCount() == 1)
			{
				_LOG(Log, "unused resource : [%s], removed\n", it->first.c_str());
				it->second->DestroyRenderThreadResource();
				it->second = nullptr;
				Resources.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}

	void TResourceLibrary::RemoveAllResources()
	{
		MapResources::iterator it = Resources.begin();
		for (; it != Resources.end(); ++ it)
		{
			it->second->DestroyRenderThreadResource();
			it->second = nullptr;
		}
		Resources.clear();
	}
}