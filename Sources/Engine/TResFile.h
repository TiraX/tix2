/*
	TiX Engine v2.0 Copyright (C) 2018
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
		bool Load(TFile& res_file);

		TResourcePtr CreateResource();

		//TNodeStaticMesh* CreateStaticMesh(TNode* ParentNode = nullptr);
		TMeshBufferPtr CreateMeshBuffer();
		TTexturePtr CreateTexture();

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

	protected:
		TString Filename;
		int8* Filebuffer;

		const TResfileChunkHeader* ChunkHeader[ECL_COUNT];
		TResfileHeader* Header;
		int32* StringOffsets;

		friend class TResourceLibrary;
	};
	typedef TI_INTRUSIVE_PTR(TResFile) TResFilePtr;
}