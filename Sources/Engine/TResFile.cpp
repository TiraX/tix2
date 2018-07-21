/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "TResFile.h"

namespace tix
{
	TResFile::TResFile()
		: Filebuffer(nullptr)
		, Header(nullptr)
		, StringOffsets(nullptr)
	{
		memset(ChunkHeader, 0, sizeof(ChunkHeader));
	}

	TResFile::~TResFile()
	{
		Destroy();
	}

	void TResFile::Destroy()
	{
		Filename = "";
		SAFE_DELETE_ARRAY(Filebuffer);
	}

	TFile* TResFile::OpenResFile(const TString& Filename)
	{
		TFile* file = ti_new TFile;
		if (!file->Open(Filename, EFA_READ))
		{
			ti_delete file;
			return nullptr;
		}
		return file;
	}

	bool TResFile::Load(const TString& Filename)
	{
		TFile* file = OpenResFile(Filename);
		if (file == nullptr)
			return false;

		bool result = Load(*file);
		ti_delete file;
		return result;
	}

	bool TResFile::Load(TFile& res_file)
	{
		Filename = res_file.GetFileName();
		TI_ASSERT(Filebuffer == nullptr);
		Filebuffer = ti_new int8[res_file.GetSize()];
		res_file.Read(Filebuffer, res_file.GetSize(), res_file.GetSize());
		res_file.Close();

		int32 pos = 0;
		Header = (TResfileHeader*)(Filebuffer + pos);
		if (Header->Version != TIRES_VERSION_MAINFILE)
		{
			TI_ASSERT(0);
			_LOG(Error, "Wrong file version. [%s]\n", Filename.c_str());
			return false;
		}
		pos += ti_align8((int32)sizeof(TResfileHeader));

		LoadStringList();

		bool result = LoadChunks(Filebuffer + pos);
		if (!result)
			Destroy();
		return result;
	}

	bool TResFile::LoadStringList()
	{
		StringOffsets = (int32*)(Filebuffer + Header->StringOffset);
		return true;
	}

	const int8* TResFile::GetString(int32 str_index)
	{
		TI_ASSERT(str_index >= 0 && str_index < Header->StringCount);
		return (char*)(StringOffsets + Header->StringCount) + (str_index > 0 ? StringOffsets[str_index - 1] : 0);
	}

	bool TResFile::LoadChunks(const char* chunk_start)
	{
		const TResfileChunkHeader* chunkHeader;
		for (int32 c = 0 ; c < Header->ChunkCount ; ++ c)
		{
			chunkHeader = (const TResfileChunkHeader*)chunk_start;

			switch (chunkHeader->ID)
			{
			case TIRES_ID_CHUNK_MESH:
				ChunkHeader[ECL_MESHES] = chunkHeader;
				TI_ASSERT(chunkHeader->Version == TIRES_VERSION_CHUNK_MESH);
				break;
			case TIRES_ID_CHUNK_TEXTURE:
				ChunkHeader[ECL_TEXTURES] = chunkHeader;
				TI_ASSERT(chunkHeader->Version == TIRES_VERSION_CHUNK_TEXTURE);
				break;
			default:
				TI_ASSERT(0);
				break;
			}
			chunk_start		+= chunkHeader->ChunkSize;
		}
		return true;
	}

	TResourcePtr TResFile::CreateResource()
	{
		TResourcePtr Resource;
		if (ChunkHeader[ECL_MESHES] != nullptr)
			Resource = CreateMeshBuffer();

		if (ChunkHeader[ECL_TEXTURES] != nullptr)
			Resource = CreateTexture();

#ifdef TI_DEBUG
		Resource->ResourceName = Filename;
#endif

		return Resource;
	}

	TMeshBufferPtr TResFile::CreateMeshBuffer()
	{
		if (ChunkHeader[ECL_MESHES] == nullptr)
			return nullptr;

		const int8* ChunkStart = (const int8*)ChunkHeader[ECL_MESHES];
		const int32 MeshCount = ChunkHeader[ECL_MESHES]->ElementCount;
		if (MeshCount == 0)
		{
			return nullptr;
		}

		TVector<TNodeStaticMesh*> MeshList;
		MeshList.reserve(MeshCount);

		TMeshBufferPtr Result;

		const int8* MeshDataStart = (const int8*)(ChunkStart + ti_align8((int32)sizeof(TResfileChunkHeader)));
		const int8* VertexDataStart = MeshDataStart + ti_align8((int32)sizeof(THeaderMesh)) * MeshCount;
		int32 MeshDataOffset = 0;
		for (int32 i = 0; i < MeshCount; ++i)
		{
			const THeaderMesh* Header = (const THeaderMesh*)(MeshDataStart + ti_align8((int32)sizeof(THeaderMesh)) * i);
			TMeshBufferPtr Mesh = ti_new TMeshBuffer();

			const int32 VertexStride = TMeshBuffer::GetStrideFromFormat(Header->VertexFormat);
			const int8* VertexData = VertexDataStart + MeshDataOffset;
			const int8* IndexData = VertexDataStart + ti_align8(Header->VertexCount * VertexStride);
			Mesh->SetVertexStreamData(Header->VertexFormat, VertexData, Header->VertexCount, (E_INDEX_TYPE)Header->IndexType, IndexData, Header->PrimitiveCount * 3);
			Mesh->SetBBox(Header->BBox);

			Result = Mesh;
		}
		return Result;
	}

	TTexturePtr TResFile::CreateTexture()
	{
		if (ChunkHeader[ECL_TEXTURES] == nullptr)
			return nullptr;

		const uint8* ChunkStart = (const uint8*)ChunkHeader[ECL_TEXTURES];
		const int32 TextureCount = ChunkHeader[ECL_TEXTURES]->ElementCount;
		if (TextureCount == 0)
		{
			return nullptr;
		}

		const uint8* HeaderStart = (const uint8*)(ChunkStart + ti_align8((int32)sizeof(TResfileChunkHeader)));
		const uint8* TextureDataStart = HeaderStart + ti_align8((int32)sizeof(THeaderTexture)) * TextureCount;

		TTexturePtr Result;
		// each ResFile should have only 1 resource
		for (int32 i = 0; i < TextureCount; ++i)
		{
			const THeaderTexture* Header = (const THeaderTexture*)(HeaderStart + ti_align8((int32)sizeof(THeaderTexture)) * i);
			TTexturePtr Texture = ti_new TTexture;
			Texture->Desc = Header->Desc;

			int32 DataOffset = 0;
			for (uint32 m = 0; m < Texture->Desc.Mips; ++m)
			{
				const uint8* Data = TextureDataStart + DataOffset;
				int32 Width = *(const int32*)(Data + sizeof(int32) * 0);
				int32 Height = *(const int32*)(Data + sizeof(int32) * 1);
				int32 Size = *(const int32*)(Data + sizeof(int32) * 2);
				Texture->AddSurface(Width, Height, Data + sizeof(uint32) * 3, Size);
				DataOffset += Size + sizeof(uint32) * 3;
				DataOffset = ti_align4(DataOffset);
			}

			Result = Texture;
		}
		return Result;
	}
}
