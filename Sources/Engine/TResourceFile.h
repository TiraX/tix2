/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "TResourceFileDef.h"

namespace tix
{
	class TResourceFile : public IReferenceCounted
	{
	private:
		TResourceFile();
		virtual ~TResourceFile();

		bool Load(const TString& InFilename);

		void CreateResource(TVector<TResourcePtr>& OutResources);

		void LoadScene();
		void CreateMeshBuffer(TVector<TResourcePtr>& OutResources);
		void CreateTexture(TVector<TResourcePtr>& OutResources);
		void CreateMaterial(TVector<TResourcePtr>& OutResources);
		void CreateMaterialInstance(TVector<TResourcePtr>& OutResources);

		const TString& GetFilename()
		{
			return Filename;
		}

		TI_API const int8* GetString(int32 StringIndex);

	private:
		void Destroy();
		TFile * OpenResFile(const TString& file_name);
		bool LoadChunks(const char* chunk_start);
		bool LoadStringList();
		bool ReadFile(const TString& InFilename);
		bool ParseFile();

	protected:
		TString Filename;
		int8* Filebuffer;

		const TResfileChunkHeader* ChunkHeader[ECL_COUNT];
		TResfileHeader* Header;
		int32* StringOffsets;

		friend class TResourceLibrary;
		friend class TResourceLoadingTask;
	};
}