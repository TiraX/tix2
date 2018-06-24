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
	public:
		TResFile();
		virtual ~TResFile();

		TFile* OpenResFile(const TString& file_name);
		bool Load(const TString& InFilename);
		bool Load(TFile& res_file);

		const TString& GetFilename()
		{
			return Filename;
		}

		void Destroy();
		bool LoadChunks(const char* chunk_start);

		bool LoadStringList();
		const int8* GetString(int32 StringIndex);

	protected:
		TString Filename;
		int8* Filebuffer;

		const TResfileChunkHeader* ChunkHeader[ECL_COUNT];
		TResfileHeader* Header;
		int32* StringOffsets;
	};

	typedef TI_INTRUSIVE_PTR(TResFile) TResFilePtr;
}