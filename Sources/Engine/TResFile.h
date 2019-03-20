/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "TResfileDef.h"

namespace tix
{
	class TResFile : public IReferenceCounted
	{
	private:
		TResFile();
		virtual ~TResFile();

		bool Load(const TString& InFilename);

		TResourcePtr CreateResource();

		void LoadScene();
		TMeshBufferPtr CreateMeshBuffer();
		TTexturePtr CreateTexture();
		TMaterialPtr CreateMaterial();
		TMaterialInstancePtr CreateMaterialInstance();

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