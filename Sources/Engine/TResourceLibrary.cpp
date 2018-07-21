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

	void TResourceLibrary::RemoveUnusedResouces()
	{
		TI_ASSERT(0);
		TI_TODO("TResourceLibrary remove unused resources implementation.");
		//		MapTextures::iterator it	= Textures.begin();
		//		for ( ; it != Textures.end() ; )
		//		{
		//			if ( it->second->referenceCount() == 1)
		//			{
		//#ifdef TI_PLATFORM_WIN32
		//				TiTexturePtr texture	= it->second;
		//				TextureData		-= texture->GetWidth() * texture->GetHeight() * 4;
		//#endif
		//				_LOG("[unused texture] : %s\n", it->first.c_str());
		//				it->second	= NULL;
		//				Textures.erase( it ++ );
		//			}
		//			else
		//			{
		//				++ it;
		//			}
		//		}
	}
}