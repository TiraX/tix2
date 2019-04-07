/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TAssetFile.h"

namespace tix
{
	// These lines are coded in Stanford~
	// Refactor all loading processes, pre-create all game thread resources with empty ones.
	// Analysis the dependences with different resources and loading them by order.
	TI_TODO("Refactor resource loadings.");

	TAssetFile::TAssetFile()
		: Filebuffer(nullptr)
		, Header(nullptr)
		, StringOffsets(nullptr)
	{
		memset(ChunkHeader, 0, sizeof(ChunkHeader));
	}

	TAssetFile::~TAssetFile()
	{
		Destroy();
	}

	void TAssetFile::Destroy()
	{
		Filename = "";
		SAFE_DELETE_ARRAY(Filebuffer);
	}

	TFile* TAssetFile::OpenResFile(const TString& Filename)
	{
		TFile* file = ti_new TFile;
		if (!file->Open(Filename, EFA_READ))
		{
			ti_delete file;
			return nullptr;
		}
		return file;
	}

	bool TAssetFile::Load(const TString& InFilename)
	{
		if (!ReadFile(InFilename))
		{
			return false;
		}
		return ParseFile();
	}

	bool TAssetFile::ReadFile(const TString& InFilename)
	{
		TFile* File = OpenResFile(InFilename);
		if (File == nullptr)
		{
			TString Path = TPath::GetAbsolutePath(InFilename);
			File = OpenResFile(Path);
		}
		if (File == nullptr)
		{
			return false;
		}
		Filename = File->GetFileName();
		TI_ASSERT(Filebuffer == nullptr);
		Filebuffer = ti_new int8[File->GetSize()];
		File->Read(Filebuffer, File->GetSize(), File->GetSize());
		File->Close();
		ti_delete File;
		return true;
	}

	bool TAssetFile::ParseFile()
	{
		int32 pos = 0;
		Header = (TResfileHeader*)(Filebuffer + pos);
		if (Header->Version != TIRES_VERSION_MAINFILE)
		{
			TI_ASSERT(0);
			_LOG(Error, "Wrong file version. [%s]\n", Filename.c_str());
			return false;
		}
		pos += ti_align4((int32)sizeof(TResfileHeader));

		LoadStringList();

		bool result = LoadChunks(Filebuffer + pos);
		if (!result)
			Destroy();
		return result;
	}

	bool TAssetFile::LoadStringList()
	{
		StringOffsets = (int32*)(Filebuffer + Header->StringOffset);
		return true;
	}

	const int8* TAssetFile::GetString(int32 str_index)
	{
		TI_ASSERT(str_index >= 0 && str_index < Header->StringCount);
		return (char*)(StringOffsets + Header->StringCount) + (str_index > 0 ? StringOffsets[str_index - 1] : 0);
	}

	bool TAssetFile::LoadChunks(const char* chunk_start)
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
			case TIRES_ID_CHUNK_MATERIAL:
				ChunkHeader[ECL_MATERIAL] = chunkHeader;
				TI_ASSERT(chunkHeader->Version == TIRES_VERSION_CHUNK_MATERIAL);
				break;
			case TIRES_ID_CHUNK_MINSTANCE:
				ChunkHeader[ECL_MATERIAL_INSTANCE] = chunkHeader;
				TI_ASSERT(chunkHeader->Version == TIRES_VERSION_CHUNK_MINSTANCE);
				break;
			case TIRES_ID_CHUNK_SCENE:
				ChunkHeader[ECL_SCENE] = chunkHeader;
				TI_ASSERT(chunkHeader->Version == TIRES_VERSION_CHUNK_SCENE);
				break;
			default:
				TI_ASSERT(0);
				break;
			}
			chunk_start += chunkHeader->ChunkSize;
		}
		return true;
	}

	void TAssetFile::CreateResource(TVector<TResourcePtr>& OutResources)
	{
		if (ChunkHeader[ECL_MESHES] != nullptr)
			CreateMeshBuffer(OutResources);

		if (ChunkHeader[ECL_TEXTURES] != nullptr)
			CreateTexture(OutResources);

		if (ChunkHeader[ECL_MATERIAL] != nullptr)
			CreateMaterial(OutResources);

		if (ChunkHeader[ECL_MATERIAL_INSTANCE] != nullptr)
			CreateMaterialInstance(OutResources);

		for (auto& Res : OutResources)
		{
			Res->SetResourceName(Filename);
		}
	}

	void TAssetFile::CreateMeshBuffer(TVector<TResourcePtr>& OutResources)
	{
		if (ChunkHeader[ECL_MESHES] == nullptr)
			return;

		const int8* ChunkStart = (const int8*)ChunkHeader[ECL_MESHES];
		const int32 MeshCount = ChunkHeader[ECL_MESHES]->ElementCount;
		if (MeshCount == 0)
		{
			return;
		}
		
		OutResources.reserve(MeshCount);
		const int8* MeshDataStart = (const int8*)(ChunkStart + ti_align4((int32)sizeof(TResfileChunkHeader)));
		const int8* VertexDataStart = MeshDataStart + ti_align4((int32)sizeof(THeaderMesh)) * MeshCount;
		int32 MeshDataOffset = 0;
		for (int32 i = 0; i < MeshCount; ++i)
		{
			const THeaderMesh* Header = (const THeaderMesh*)(MeshDataStart + ti_align4((int32)sizeof(THeaderMesh)) * i);
			TMeshBufferPtr Mesh = ti_new TMeshBuffer();

			// Load vertex data and index data
			const int32 VertexStride = TMeshBuffer::GetStrideFromFormat(Header->VertexFormat);
			const int8* VertexData = VertexDataStart + MeshDataOffset;
			const int8* IndexData = VertexDataStart + ti_align4(Header->VertexCount * VertexStride);
			Mesh->SetVertexStreamData(Header->VertexFormat, VertexData, Header->VertexCount, (E_INDEX_TYPE)Header->IndexType, IndexData, Header->PrimitiveCount * 3);
			Mesh->SetBBox(Header->BBox);

			// Load material
			TString MaterialResName = GetString(Header->StrMaterialInstance);
			if (MaterialResName.find(".tres") == TString::npos)
			{
				MaterialResName += ".tres";
			}
			TAssetPtr MIRes = TAssetLibrary::Get()->LoadAsset(MaterialResName);
			if (MIRes == nullptr)
			{
				_LOG(Error, "Failed to load default material instance [%s] for mesh [%s].\n", MaterialResName.c_str(), Filename.c_str());
			}
			TMaterialInstancePtr MaterialInstance = static_cast<TMaterialInstance*>(MIRes->GetResourcePtr());
			Mesh->SetDefaultMaterial(MaterialInstance);

			OutResources.push_back(Mesh);
		}
	}

	void TAssetFile::CreateTexture(TVector<TResourcePtr>& OutResources)
	{
		if (ChunkHeader[ECL_TEXTURES] == nullptr)
			return;

		const uint8* ChunkStart = (const uint8*)ChunkHeader[ECL_TEXTURES];
		const int32 TextureCount = ChunkHeader[ECL_TEXTURES]->ElementCount;
		if (TextureCount == 0)
		{
			return;
		}

		const uint8* HeaderStart = (const uint8*)(ChunkStart + ti_align4((int32)sizeof(TResfileChunkHeader)));
		const uint8* TextureDataStart = HeaderStart + ti_align4((int32)sizeof(THeaderTexture)) * TextureCount;

		// each ResFile should have only 1 resource
		OutResources.reserve(TextureCount);
		for (int32 i = 0; i < TextureCount; ++i)
		{
			const THeaderTexture* Header = (const THeaderTexture*)(HeaderStart + ti_align4((int32)sizeof(THeaderTexture)) * i);

			TTextureDesc Desc;
			Desc.Type = (E_TEXTURE_TYPE)Header->Type;
			Desc.Format = (E_PIXEL_FORMAT)Header->Format;
			Desc.Width = Header->Width;
			Desc.Height = Header->Height;
			Desc.AddressMode = (E_TEXTURE_ADDRESS_MODE)Header->AddressMode;
			Desc.SRGB = Header->SRGB;
			Desc.Mips = Header->Mips;

			if (Desc.SRGB != 0)
			{
				Desc.Format = GetSRGBFormat(Desc.Format);
			}

			TTexturePtr Texture = ti_new TTexture(Desc);

			int32 ArraySize = 1;
			if (Desc.Type == ETT_TEXTURE_CUBE)
			{
				ArraySize = 6;
			}
			TI_ASSERT(Header->Surfaces == ArraySize * Desc.Mips);

			int32 DataOffset = 0;
			for (int32 a = 0; a < ArraySize; ++a)
			{
				for (uint32 m = 0; m < Texture->GetDesc().Mips; ++m)
				{
					const uint8* Data = TextureDataStart + DataOffset;
					int32 Width = *(const int32*)(Data + sizeof(int32) * 0);
					int32 Height = *(const int32*)(Data + sizeof(int32) * 1);
					int32 RowPitch = *(const int32*)(Data + sizeof(int32) * 2);
					int32 Size = *(const int32*)(Data + sizeof(int32) * 3);
					Texture->AddSurface(Width, Height, Data + sizeof(uint32) * 4, RowPitch, Size);
					DataOffset += Size + sizeof(uint32) * 4;
					DataOffset = ti_align4(DataOffset);
				}
			}

			OutResources.push_back(Texture);
		}
	}

	void TAssetFile::CreateMaterial(TVector<TResourcePtr>& OutResources)
	{
		if (ChunkHeader[ECL_MATERIAL] == nullptr)
			return;

		const uint8* ChunkStart = (const uint8*)ChunkHeader[ECL_MATERIAL];
		const int32 MaterialCount = ChunkHeader[ECL_MATERIAL]->ElementCount;
		if (MaterialCount == 0)
		{
			return;
		}

		const uint8* HeaderStart = (const uint8*)(ChunkStart + ti_align4((int32)sizeof(TResfileChunkHeader)));
		//const uint8* CodeDataStart = HeaderStart + ti_align4((int32)sizeof(THeaderMaterial)) * MaterialCount;
		
		OutResources.reserve(MaterialCount);
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			const THeaderMaterial* Header = (const THeaderMaterial*)(HeaderStart + ti_align4((int32)sizeof(THeaderMaterial)) * i);
			TMaterialPtr Material = ti_new TMaterial;

			// Load material
			TShaderNames ShaderNames;
			for (int32 s = 0; s < ESS_COUNT; ++s)
			{
				ShaderNames.ShaderNames[s] = GetString(Header->ShaderNames[s]);
			}

			Material->EnableState(EPSO_BLEND, (Header->Flags & EPSO_BLEND) != 0);
			Material->EnableState(EPSO_DEPTH, (Header->Flags & EPSO_DEPTH) != 0);
			Material->EnableState(EPSO_DEPTH_TEST, (Header->Flags & EPSO_DEPTH_TEST) != 0);
			Material->EnableState(EPSO_STENCIL, (Header->Flags & EPSO_STENCIL) != 0);

			Material->SetShaderVsFormat(Header->VsFormat);
			Material->SetBlendState(Header->BlendState);
			Material->SetRasterizerState(Header->RasterizerDesc);
			Material->SetDepthStencilState(Header->DepthStencilDesc);
			
			// RT info
			int RTNum = 0;
			for (int32 cb = 0; cb < ERTC_COUNT; ++cb)
			{
				if (Header->ColorBuffers[cb] == EPF_UNKNOWN)
					break;
				else
				{
					Material->SetRTColor((E_PIXEL_FORMAT)Header->ColorBuffers[cb], (E_RT_COLOR_BUFFER)cb);
					++RTNum;
				}
			}
			if (Header->DepthBuffer != EPF_UNKNOWN)
			{
				Material->SetRTDepth((E_PIXEL_FORMAT)Header->DepthBuffer);
			}
			Material->SetRTColorBufferCount(RTNum);

			// Load Shader code
			int32 CodeOffset = 0;
			TShaderPtr Shader = ti_new TShader(ShaderNames);
			if (Header->ShaderCodeLength[0] > 0)
			{
				// Load from res file
				//Material->SetShaderCode((E_SHADER_STAGE)s, CodeDataStart + CodeOffset, Header->ShaderCodeLength[s]);
				TI_ASSERT(0);
			}
			else
			{
				Shader->LoadShaderCode();
			}
			Material->SetShader(Shader);
			OutResources.push_back(Material);
		}
	}

	void TAssetFile::CreateMaterialInstance(TVector<TResourcePtr>& OutResources)
	{
		if (ChunkHeader[ECL_MATERIAL_INSTANCE] == nullptr)
			return;

		const uint8* ChunkStart = (const uint8*)ChunkHeader[ECL_MATERIAL_INSTANCE];
		const int32 MICount = ChunkHeader[ECL_MATERIAL_INSTANCE]->ElementCount;
		if (MICount == 0)
		{
			return;
		}

		const uint8* HeaderStart = (const uint8*)(ChunkStart + ti_align4((int32)sizeof(TResfileChunkHeader)));
		const uint8* MIDataStart = HeaderStart + ti_align4((int32)sizeof(THeaderMaterialInstance)) * MICount;

		OutResources.reserve(MICount);
		// each ResFile should have only 1 resource
		for (int32 i = 0; i < MICount; ++i)
		{
			const THeaderMaterialInstance* Header = (const THeaderMaterialInstance*)(HeaderStart + ti_align4((int32)sizeof(THeaderMaterialInstance)) * i);
			TMaterialInstancePtr MInstance = ti_new TMaterialInstance;
			
			MInstance->ParamNames.reserve(Header->ParamCount);
			MInstance->ParamTypes.reserve(Header->ParamCount);

			const int32* ParamNameOffset = (const int32*)(MIDataStart + 0);
			const uint8* ParamTypeOffset = (const uint8*)(MIDataStart + sizeof(int32) * Header->ParamCount);
			const uint8* ParamValueOffset = (const uint8*)(MIDataStart + sizeof(int32) * Header->ParamCount + ti_align4(Header->ParamCount));

			int32 TotalValueBufferLength = 0;
			for (int32 p = 0; p < Header->ParamCount; ++p)
			{
				E_MI_PARAM_TYPE ParamType = (E_MI_PARAM_TYPE)(ParamTypeOffset[p]);
				const int32 ValueBytes = TMaterialInstance::GetParamTypeBytes(ParamType);
				if (ParamType != MIPT_TEXTURE)
				{
					TotalValueBufferLength += ValueBytes;
				}
			}
			if (TotalValueBufferLength > 0)
			{
				MInstance->ParamValueBuffer = ti_new TStream(TotalValueBufferLength);
			}

			// Load param names and types
			int32 ValueOffset = 0;
			for (int32 p = 0; p < Header->ParamCount; ++p)
			{
				MInstance->ParamNames.push_back(GetString(ParamNameOffset[p]));
				E_MI_PARAM_TYPE ParamType = (E_MI_PARAM_TYPE)(ParamTypeOffset[p]);
				MInstance->ParamTypes.push_back(ParamType);
				const int32 ValueBytes = TMaterialInstance::GetParamTypeBytes(ParamType);
				if (ParamType == MIPT_TEXTURE)
				{
					// texture params
					int32 TextureNameIndex = *(const int32*)(ParamValueOffset + ValueOffset);
					TString TextureName = GetString(TextureNameIndex);
					TAssetPtr TextureRes = TAssetLibrary::Get()->LoadAsset(TextureName);
					if (TextureRes == nullptr)
					{
						_LOG(Error, "Failed to load texture [%s] for Material Instance [%s].\n", TextureName.c_str(), Filename.c_str());
					}
					TTexturePtr Texture = static_cast<TTexture*>(TextureRes->GetResourcePtr());
					MInstance->ParamTextures.push_back(Texture);
				}
				else
				{
					// value params
					MInstance->ParamValueBuffer->Put(ParamValueOffset + ValueOffset, ValueBytes);
				}
				ValueOffset += ValueBytes;
			}

			// Link material
			TString MaterialResName = GetString(Header->LinkedMaterialIndex);
			if (MaterialResName.find(".tres") == TString::npos)
			{
				MaterialResName += ".tres";
			}
			TAssetPtr Material = TAssetLibrary::Get()->LoadAsset(MaterialResName);
			if (Material == nullptr)
			{
				_LOG(Error, "Failed to load material [%s] for Material Instance [%s].\n", MaterialResName.c_str(), Filename.c_str());
			}
			MInstance->LinkedMaterial = static_cast<TMaterial*>(Material->GetResourcePtr());

			OutResources.push_back(MInstance);
		}
	}

	void TAssetFile::LoadScene()
	{
		if (ChunkHeader[ECL_SCENE] == nullptr)
		{
			_LOG(Error, "Can not find scene chunk when loading scene %s.", Filename.c_str());
			return;
		}

		const uint8* ChunkStart = (const uint8*)ChunkHeader[ECL_SCENE];
		TI_ASSERT(ChunkHeader[ECL_SCENE]->ElementCount == 1);

		const uint8* HeaderStart = (const uint8*)(ChunkStart + ti_align4((int32)sizeof(TResfileChunkHeader)));
		const uint8* SceneDataStart = HeaderStart + ti_align4((int32)sizeof(THeaderScene)) * 1;

		TMaterialInstancePtr Result;
		// each ResFile should have only 1 resource
		for (int32 i = 0; i < 1; ++i)
		{
			const THeaderScene* Header = (const THeaderScene*)(HeaderStart);

			// Setup Env Node
			TNodeEnvironment* Env = TEngine::Get()->GetScene()->GetEnvironment();
			Env->SetMainLightDirection(Header->MainLightDirection);
			Env->SetMainLightColor(Header->MainLightColor);
			Env->SetMainLightIntensity(Header->MainLightIntensity);

			// Load assets names
			const int32* AssetsTextures = (const int32*)(SceneDataStart);
			const int32* AssetsMaterialInstances = AssetsTextures + Header->NumTextures;
			const int32* AssetsMaterials = AssetsMaterialInstances + Header->NumMaterialInstances;
			const int32* AssetsMeshes = AssetsMaterials + Header->NumMaterials;
			const int32* InstancesCountList = AssetsMeshes + Header->NumMeshes;

			// Do Test
			for (int32 m = 0; m < Header->NumMeshes; ++m)
			{
				_LOG(Log, "mesh : %s\n", GetString(AssetsMeshes[m]));
			}

			// Send Assets to loading thread

			// Create instance nodes
			const THeaderSceneMeshInstance* HeaderMeshInstances = (const THeaderSceneMeshInstance*)(InstancesCountList + Header->NumMeshes);
			int32 TotalInstances = 0;
			for (int32 m = 0; m < Header->NumMeshes; ++m)
			{
				int32 NumInstances = InstancesCountList[m];
				for (int32 ins = 0 ; ins < NumInstances; ++ ins)
				{
					const THeaderSceneMeshInstance& InstanceInfo = HeaderMeshInstances[TotalInstances + ins];
					//_LOG(Log, "Loading mesh ins : %f, %f, %f.\n", InstanceInfo.Position.X, InstanceInfo.Position.Y, InstanceInfo.Position.Z);
				}

				TotalInstances += NumInstances;
				TI_ASSERT(TotalInstances <= Header->NumInstances);
			}
		}
	}
}
