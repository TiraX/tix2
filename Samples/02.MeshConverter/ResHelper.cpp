/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include <fstream>

namespace tix
{
	TResFileHelper::TResFileHelper()
	{
	}

	TResFileHelper::~TResFileHelper()
	{
	}

	TStream& TResFileHelper::GetChunk(E_CHUNK_LIB ChunkType)
	{
		return ChunkStreams[ChunkType];
	}

	bool TResFileHelper::SaveFile(const TString& Filename)
	{
		// Header
		TResfileHeader HeaderResfile;
		HeaderResfile.ID = TIRES_ID_RESFILE;
		HeaderResfile.Version = TIRES_VERSION_MAINFILE;
		int32 Chunks = 0;
		for (int32 c = 0 ; c < ECL_COUNT ; ++ c)
		{
			if (ChunkStreams[c].GetLength() > 0)
			{
				++Chunks;
			}
		}
		HeaderResfile.ChunkCount = Chunks;
		HeaderResfile.FileSize = ti_align4((int32)sizeof(TResfileHeader));
		for (int32 c = 0; c < ECL_COUNT ; ++ c)
		{
			if (ChunkStreams[c].GetLength() > 0)
			{
				HeaderResfile.FileSize += ChunkStreams[c].GetLength();
			}
		}
		HeaderResfile.StringCount = (int32)Strings.size();
		HeaderResfile.StringOffset = HeaderResfile.FileSize;

		// Strings
		TStream StringStream;
		SaveStringList(Strings, StringStream);
		HeaderResfile.FileSize += StringStream.GetLength();

		TStream ChunkHeaderStream;
		ChunkHeaderStream.Put(&HeaderResfile, sizeof(TResfileHeader));
		FillZero4(ChunkHeaderStream);

		// Write to file
		TFile file;
		if (file.Open(Filename, EFA_CREATEWRITE))
		{
			// header
			file.Write(ChunkHeaderStream.GetBuffer(), ChunkHeaderStream.GetLength());

			// chunk
			for (int32 c = 0; c < ECL_COUNT; ++c)
			{
				if (ChunkStreams[c].GetLength() > 0)
				{
					file.Write(ChunkStreams[c].GetBuffer(), ChunkStreams[c].GetLength());
				}
			}

			// strings
			file.Write(StringStream.GetBuffer(), StringStream.GetLength());
			file.Close();
			return true;
		}

		return false;
	}
}
