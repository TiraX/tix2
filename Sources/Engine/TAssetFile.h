/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "TAssetFileDef.h"

namespace tix
{
	class TAssetFile : public IReferenceCounted
	{
	public:
		TAssetFile();
		~TAssetFile();

		const TString& GetFilename()
		{
			return Filename;
		}

		bool Load(const TString& InFilename);
		void CreateResource(TVector<TResourcePtr>& OutResources);

		void LoadScene();

		bool ReadFile(const TString& InFilename);
		bool ParseFile();

	private:
		void CreateMeshBuffer(TVector<TResourcePtr>& OutResources);
		void CreateTexture(TVector<TResourcePtr>& OutResources);
		void CreateMaterial(TVector<TResourcePtr>& OutResources);
		void CreateMaterialInstance(TVector<TResourcePtr>& OutResources);
		
		TI_API const int8* GetString(int32 StringIndex);

		void Destroy();
		TFile * OpenResFile(const TString& file_name);
		bool LoadChunks(const char* chunk_start);
		bool LoadStringList();

	protected:
		TString Filename;
		int8* Filebuffer;

		const TResfileChunkHeader* ChunkHeader[ECL_COUNT];
		TResfileHeader* Header;
		int32* StringOffsets;

		friend class TAssetLibrary;
		friend class TResourceLoadingTask;
	};
}