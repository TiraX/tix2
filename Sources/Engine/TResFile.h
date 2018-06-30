/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "TResfileDef.h"

namespace tix
{
	class TResFile;
	typedef TI_INTRUSIVE_PTR(TResFile) TResFilePtr;

	class TNode;
	class TNodeStaticMesh;

	class TResFile : public IReferenceCounted
	{
	public:
		TResFile();
		virtual ~TResFile();

		static TI_API TResFilePtr LoadResfile(const TString& InFilename);

		TI_API bool Load(const TString& InFilename);
		TI_API bool Load(TFile& res_file);

		TI_API TNodeStaticMesh* CreateStaticMesh(TNode* ParentNode = nullptr);

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
	};
}