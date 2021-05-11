/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TAssetFile.h"

namespace tix
{
	// These lines are coded in Stanford~
	// Refactor all loading processes, pre-create all game thread resources with empty ones.
	// Analysis the dependences with different resources and loading them by order.

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
			_LOG(Error, "Failed to read file %s.\n", InFilename.c_str());
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
			_LOG(Error, "Failed to LoadAsset %s.\n", InFilename.c_str());
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
		pos += TMath::Align4((int32)sizeof(TResfileHeader));

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
			case TIRES_ID_CHUNK_SCENETILE:
				ChunkHeader[ECL_SCENETILE] = chunkHeader;
				TI_ASSERT(chunkHeader->Version == TIRES_VERSION_CHUNK_SCENETILE);
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

		if (ChunkHeader[ECL_SCENE] != nullptr)
			CreateScene();

		if (ChunkHeader[ECL_SCENETILE] != nullptr)
			CreateSceneTile(OutResources);

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
		
		TI_TODO("Maybe we dont need TVector<TResourcePtr> to hold multi res. ONLY return 1 resource");
		// New mesh format should have 1 mesh buffer ONLY, and multiple mesh sections
		TI_ASSERT(MeshCount == 1);
		// Mesh sections and 1 collision
		OutResources.reserve(MeshCount + 1);

		// Load meshes
		const int8* MeshDataStart = (const int8*)(ChunkStart + TMath::Align4((int32)sizeof(TResfileChunkHeader)));
		const int8* SectionDataStart = (const int8*)(MeshDataStart + TMath::Align4((int32)sizeof(THeaderMesh)));

		const THeaderMesh* Header = (const THeaderMesh*)(MeshDataStart);
		const THeaderMeshSection * HeaderSections = (const THeaderMeshSection*)(SectionDataStart);

		TI_ASSERT(Header->Sections > 0);
		const int8* VertexDataStart = MeshDataStart + TMath::Align4((int32)sizeof(THeaderMesh)) * MeshCount + sizeof(THeaderMeshSection) * Header->Sections + sizeof(THeaderCollisionSet);

		// Create mesh buffer resource
		TMeshBufferPtr MeshBuffer = ti_new TMeshBuffer();
		MeshBuffer->SetResourceName(Filename + "-MB");
		TStaticMeshPtr StaticMesh = ti_new TStaticMesh(MeshBuffer);

		// Load vertex data and index data
		const int32 IndexStride = (Header->IndexType == EIT_16BIT) ? sizeof(uint16) : sizeof(uint32);
		const int32 VertexStride = TMeshBuffer::GetStrideFromFormat(Header->VertexFormat);
		const int8* VertexData = VertexDataStart;
		const int8* IndexData = VertexDataStart + TMath::Align4(Header->VertexCount * VertexStride);
		const int8* ClusterData = IndexData + TMath::Align4(Header->PrimitiveCount * 3 * IndexStride);
		MeshBuffer->SetVertexStreamData(Header->VertexFormat, VertexData, Header->VertexCount, (E_INDEX_TYPE)Header->IndexType, IndexData, Header->PrimitiveCount * 3);
		if (Header->Clusters > 0)
		{
			MeshBuffer->SetClusterData(ClusterData, Header->Clusters);
		}
		MeshBuffer->SetBBox(Header->BBox);
		TI_ASSERT(Header->ClusterSize == 0 || (Header->PrimitiveCount == Header->Clusters * Header->ClusterSize));

		FStats::Stats.VertexDataInBytes += Header->VertexCount * VertexStride;
		FStats::Stats.IndexDataInBytes += TMath::Align4((int32)(IndexStride * Header->PrimitiveCount * 3));

		// Load sections
		for (int32 s = 0 ; s < Header->Sections ; ++ s)
		{
			const THeaderMeshSection& HeaderSection = HeaderSections[s];

			TMeshSection MeshSection;
			MeshSection.IndexStart = HeaderSection.IndexStart;
			MeshSection.Triangles = HeaderSection.Triangles;

			// Load material
			TString MaterialResName = GetString(HeaderSection.StrMaterialInstance);
			TAssetPtr MIRes = TAssetLibrary::Get()->LoadAsset(MaterialResName);
			if (MIRes->GetResources().size() == 0)
			{
				_LOG(Error, "Failed to load default material instance [%s] for mesh [%s].\n", MaterialResName.c_str(), Filename.c_str());
			}
			TMaterialInstancePtr MaterialInstance = static_cast<TMaterialInstance*>(MIRes->GetResourcePtr());
			MeshSection.DefaultMaterial = MaterialInstance;

			StaticMesh->AddMeshSection(MeshSection);
		}

		// Load collisions
		{
			const int8* CollisionHeaderStart = MeshDataStart + sizeof(THeaderMesh) * MeshCount + sizeof(THeaderMeshSection) * Header->Sections;
			const int8* CollisionDataStart = VertexDataStart + 
				TMath::Align4(Header->VertexCount * VertexStride)	// Vertex data size
				+ TMath::Align4((int32)(IndexStride * Header->PrimitiveCount * 3))		// Index data size
				+ TMath::Align4((int32)(Header->Clusters * sizeof(TMeshClusterDef)));	// Cluster data size
			THeaderCollisionSet * HeaderCollision = (THeaderCollisionSet*)CollisionHeaderStart;

			const int8* CollisionSphereData = CollisionDataStart;
			const int8* CollisionBoxData = CollisionSphereData + HeaderCollision->SpheresSizeInBytes;
			const int8* CollisionCapsuleData = CollisionBoxData + HeaderCollision->BoxesSizeInBytes;
			const int8* CollisionConvexCount = CollisionCapsuleData + HeaderCollision->CapsulesSizeInBytes;
			const int8* CollisionConvexData = CollisionConvexCount + sizeof(vector2du) * HeaderCollision->NumConvexes;

			TI_ASSERT(sizeof(TCollisionSet::TSphere) * HeaderCollision->NumSpheres == HeaderCollision->SpheresSizeInBytes);
			TI_ASSERT(sizeof(TCollisionSet::TBox) * HeaderCollision->NumBoxes == HeaderCollision->BoxesSizeInBytes);
			TI_ASSERT(sizeof(TCollisionSet::TCapsule) * HeaderCollision->NumCapsules == HeaderCollision->CapsulesSizeInBytes);
			TI_ASSERT(sizeof(vector2du) * HeaderCollision->NumConvexes == HeaderCollision->ConvexesSizeInBytes);

			TCollisionSetPtr CollisionSet = ti_new TCollisionSet;
			CollisionSet->Spheres.resize(HeaderCollision->NumSpheres);
			CollisionSet->Boxes.resize(HeaderCollision->NumBoxes);
			CollisionSet->Capsules.resize(HeaderCollision->NumCapsules);
			CollisionSet->Convexes.resize(HeaderCollision->NumConvexes);

			const TCollisionSet::TSphere * SphereData = (const TCollisionSet::TSphere*)CollisionSphereData;
			for (uint32 i = 0; i < HeaderCollision->NumSpheres; ++i)
			{
				CollisionSet->Spheres[i] = SphereData[i];
			}
			const TCollisionSet::TBox * BoxData = (const TCollisionSet::TBox*)CollisionBoxData;
			for (uint32 i = 0; i < HeaderCollision->NumBoxes; ++i)
			{
				CollisionSet->Boxes[i] = BoxData[i];
			}
			const TCollisionSet::TCapsule * CapsuleData = (const TCollisionSet::TCapsule*)CollisionCapsuleData;
			for (uint32 i = 0; i < HeaderCollision->NumCapsules; ++i)
			{
				CollisionSet->Capsules[i] = CapsuleData[i];
			}
			const vector2du * ConvexDataCount = (const vector2du*)CollisionConvexCount;
			uint32 ConvexDataOffset = 0;
			for (uint32 i = 0 ; i < HeaderCollision->NumConvexes ; ++i)
			{
				const uint32 VertexCount = ConvexDataCount[i].X;
				const uint32 IndexCount = ConvexDataCount[i].Y;

				CollisionSet->Convexes[i].VertexData.resize(VertexCount);
				CollisionSet->Convexes[i].IndexData.resize(IndexCount);

				memcpy(CollisionSet->Convexes[i].VertexData.data(), CollisionConvexData + ConvexDataOffset, VertexCount * sizeof(vector3df));
				ConvexDataOffset += VertexCount * sizeof(vector3df);
				memcpy(CollisionSet->Convexes[i].IndexData.data(), CollisionConvexData + ConvexDataOffset, IndexCount * sizeof(uint16));
				ConvexDataOffset += TMath::Align4(uint32(IndexCount * sizeof(uint16)));
			}
			StaticMesh->SetCollision(CollisionSet);
		}

		// Create occlude mesh from collision
		StaticMesh->CreateOccludeMeshFromCollision();

		OutResources.push_back(StaticMesh);
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

		const uint8* HeaderStart = (const uint8*)(ChunkStart + TMath::Align4((int32)sizeof(TResfileChunkHeader)));
		const uint8* TextureDataStart = HeaderStart + TMath::Align4((int32)sizeof(THeaderTexture)) * TextureCount;

		// each ResFile should have only 1 resource
		OutResources.reserve(TextureCount);
		for (int32 i = 0; i < TextureCount; ++i)
		{
			const THeaderTexture* Header = (const THeaderTexture*)(HeaderStart + TMath::Align4((int32)sizeof(THeaderTexture)) * i);

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
					DataOffset = TMath::Align4(DataOffset);
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

		const uint8* HeaderStart = (const uint8*)(ChunkStart + TMath::Align4((int32)sizeof(TResfileChunkHeader)));
		//const uint8* CodeDataStart = HeaderStart + TMath::Align4((int32)sizeof(THeaderMaterial)) * MaterialCount;
		
		OutResources.reserve(MaterialCount);
		for (int32 i = 0; i < MaterialCount; ++i)
		{
			const THeaderMaterial* Header = (const THeaderMaterial*)(HeaderStart + TMath::Align4((int32)sizeof(THeaderMaterial)) * i);
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
			Material->SetShaderInsFormat(Header->InsFormat);
			Material->SetPrimitiveType((E_PRIMITIVE_TYPE)Header->PrmitiveType);
			Material->SetBlendState((E_BLEND_MODE)Header->BlendMode, Header->BlendState);
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
			if (FVTSystem::IsEnabled())
			{
				TI_ASSERT(RTNum == 1);
				// This is not a good way. Force the second output as the uv check layer
				Material->SetRTColor(EPF_RGBA16F, ERTC_COLOR1);
				++RTNum;
			}
			if (Header->DepthBuffer != EPF_UNKNOWN)
			{
				Material->SetRTDepth((E_PIXEL_FORMAT)Header->DepthBuffer);
			}
			Material->SetRTColorBufferCount(RTNum);

			// Load Shader code
			//int32 CodeOffset = 0;
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

		const uint8* HeaderStart = (const uint8*)(ChunkStart + TMath::Align4((int32)sizeof(TResfileChunkHeader)));
		const uint8* MIDataStart = HeaderStart + TMath::Align4((int32)sizeof(THeaderMaterialInstance)) * MICount;

		OutResources.reserve(MICount);
		// each ResFile should have only 1 resource
		for (int32 i = 0; i < MICount; ++i)
		{
			const THeaderMaterialInstance* Header = (const THeaderMaterialInstance*)(HeaderStart + TMath::Align4((int32)sizeof(THeaderMaterialInstance)) * i);
			TMaterialInstancePtr MInstance = ti_new TMaterialInstance;

			const int32 TotalParamCount = Header->ParamDataCount + Header->ParamTextureCount;
			MInstance->ParamNames.reserve(TotalParamCount);
			MInstance->ParamTypes.reserve(TotalParamCount);

			const int32* ParamNameOffset = (const int32*)(MIDataStart + 0);
			const uint8* ParamTypeOffset = (const uint8*)(MIDataStart + sizeof(int32) * TotalParamCount);
			const uint8* ParamValueOffset = (const uint8*)(MIDataStart + sizeof(int32) * TotalParamCount + TMath::Align4(TotalParamCount));

			int32 TotalValueBufferLength = 0;
			for (int32 p = 0; p < TotalParamCount; ++p)
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
			MInstance->ParamTextureNames.reserve(Header->ParamTextureCount);
			MInstance->ParamTextureSizes.reserve(Header->ParamTextureCount);
			for (int32 p = 0; p < TotalParamCount; ++p)
			{
				MInstance->ParamNames.push_back(GetString(ParamNameOffset[p]));
				E_MI_PARAM_TYPE ParamType = (E_MI_PARAM_TYPE)(ParamTypeOffset[p]);
				MInstance->ParamTypes.push_back(ParamType);
				const int32 ValueBytes = TMaterialInstance::GetParamTypeBytes(ParamType);
				if (ParamType == MIPT_TEXTURE)
				{
					// texture params
					int32 TextureNameIndex = *(const int32*)(ParamValueOffset + ValueOffset);
					const int16 * TextureSize = (const int16*)(ParamValueOffset + ValueOffset + sizeof(int32));
					TString TextureName = GetString(TextureNameIndex);
					MInstance->ParamTextureNames.push_back(TextureName);
					vector2di Size = vector2di(TextureSize[0], TextureSize[1]);
					MInstance->ParamTextureSizes.push_back(Size);
					if (!FVTSystem::IsEnabled())
					{
						TAssetPtr TextureRes = TAssetLibrary::Get()->LoadAsset(TextureName);
						if (TextureRes->GetResources().size() == 0)
						{
							_LOG(Error, "Failed to load texture [%s] for Material Instance [%s].\n", TextureName.c_str(), Filename.c_str());
						}
						TTexturePtr Texture = static_cast<TTexture*>(TextureRes->GetResourcePtr());
						MInstance->ParamTextures.push_back(Texture);
					}
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
			TAssetPtr Material = TAssetLibrary::Get()->LoadAsset(MaterialResName);
			if (Material->GetResources().size() == 0)
			{
				_LOG(Error, "Failed to load material [%s] for Material Instance [%s].\n", MaterialResName.c_str(), Filename.c_str());
			}
			MInstance->LinkedMaterial = static_cast<TMaterial*>(Material->GetResourcePtr());

			OutResources.push_back(MInstance);
		}
	}

	void TAssetFile::CreateScene()
	{
		if (ChunkHeader[ECL_SCENE] == nullptr)
		{
			_LOG(Error, "Can not find scene chunk when loading scene %s.", Filename.c_str());
			return;
		}

		const uint8* ChunkStart = (const uint8*)ChunkHeader[ECL_SCENE];
		TI_ASSERT(ChunkHeader[ECL_SCENE]->ElementCount == 1);

		const uint8* HeaderStart = (const uint8*)(ChunkStart + TMath::Align4((int32)sizeof(TResfileChunkHeader)));
		const uint8* SceneDataStart = HeaderStart + TMath::Align4((int32)sizeof(THeaderScene)) * 1;

		{
			const THeaderScene* Header = (const THeaderScene*)(HeaderStart);

			// Setup Env Node
			TNodeEnvironment* Env = TEngine::Get()->GetScene()->GetEnvironment();
			Env->SetMainLightDirection(Header->MainLightDirection);
			Env->SetMainLightColor(Header->MainLightColor);
			Env->SetMainLightIntensity(Header->MainLightIntensity);
			Env->SetSkyLightSH3(Header->SkyLight_SH3);

			// Load Cameras
			if (Header->NumCameras > 0)
			{
				const THeaderCameraInfo * CamInfoData = (const THeaderCameraInfo*)(SceneDataStart);
				// Pick the first for now
				const THeaderCameraInfo& CamInfo = CamInfoData[0];
				TNodeCamera * Camera = TEngine::Get()->GetScene()->GetActiveCamera();
				Camera->SetPosition(CamInfo.Location);
				Camera->SetTarget(CamInfo.Target);
				// UE4 exported fov in horizontal direction.
				Camera->SetFOVX(TMath::DegToRad(CamInfo.FOV));
				Camera->SetAspectRatio(CamInfo.Aspect);
				Camera->SetRotator(vector3df(TMath::DegToRad(CamInfo.Rotate.X), TMath::DegToRad(CamInfo.Rotate.Y), TMath::DegToRad(CamInfo.Rotate.Z)));
			}

			// Load assets names
			const vector2di16* AssetsTiles = (const vector2di16*)(SceneDataStart + sizeof(THeaderCameraInfo) * Header->NumCameras);

			TAssetLibrary * AssetLib = TAssetLibrary::Get();
			
			// Create a level node
			TString LevelName = GetString(Header->NameIndex);
			TNodeLevel * NodeLevel = TNodeFactory::CreateNode<TNodeLevel>(TEngine::Get()->GetScene()->GetRoot(), LevelName);

			// Load scene tiles
			TVector<TAssetPtr> Tiles;
			Tiles.reserve(Header->NumTiles);
			int8 TileFileName[128];
			for (int32 t = 0; t < Header->NumTiles; ++t)
			{
				const vector2di16& Point = AssetsTiles[t];
				sprintf(TileFileName, "t%d_%d", Point.X, Point.Y);
				TString TileName = TileFileName;
				sprintf(TileFileName, "%s/t%d_%d.tasset", LevelName.c_str(), Point.X, Point.Y);
				TString TileFilePath = TileFileName;

				TSceneTileLoadingFinishDelegate * SceneTileLoadingFinishDelegate = ti_new TSceneTileLoadingFinishDelegate(LevelName, TileName);
				AssetLib->LoadAssetAysc(TileFilePath, SceneTileLoadingFinishDelegate);
			}
		}
	}

	inline void GetInstanceRotationScaleMatrix(FMatrix& OutMatrix, const quaternion& Rotation, const vector3df& Scale)
	{
		matrix4 MatInstanceTrans;
		Rotation.getMatrix(MatInstanceTrans);
		MatInstanceTrans.postScale(Scale);
		MatInstanceTrans = MatInstanceTrans.getTransposed();
		OutMatrix = MatInstanceTrans;
	}

	inline void MatrixRotationScaleToHalf3(const FMatrix& Mat, FHalf4& Rot0, FHalf4& Rot1, FHalf4& Rot2)
	{
		Rot0.X = Mat[0];
		Rot0.Y = Mat[1];
		Rot0.Z = Mat[2];
		Rot0.W = Mat[3];
		Rot1.X = Mat[4];
		Rot1.Y = Mat[5];
		Rot1.Z = Mat[6];
		Rot1.W = Mat[7];
		Rot2.X = Mat[8];
		Rot2.Y = Mat[9];
		Rot2.Z = Mat[10];
		Rot2.W = Mat[11];
	}

	inline void MatrixRotationScaleToFloat3(const FMatrix& Mat, FFloat4& Rot0, FFloat4& Rot1, FFloat4& Rot2)
	{
		Rot0.X = Mat[0];
		Rot0.Y = Mat[1];
		Rot0.Z = Mat[2];
		Rot0.W = Mat[3];
		Rot1.X = Mat[4];
		Rot1.Y = Mat[5];
		Rot1.Z = Mat[6];
		Rot1.W = Mat[7];
		Rot2.X = Mat[8];
		Rot2.Y = Mat[9];
		Rot2.Z = Mat[10];
		Rot2.W = Mat[11];
	}

	void TAssetFile::CreateSceneTile(TVector<TResourcePtr>& OutResources)
	{
		if (ChunkHeader[ECL_SCENETILE] == nullptr)
		{
			_LOG(Error, "Can not find scene tile chunk when loading scene %s.", Filename.c_str());
			return;
		}

		const uint8* ChunkStart = (const uint8*)ChunkHeader[ECL_SCENETILE];
		TI_ASSERT(ChunkHeader[ECL_SCENETILE]->ElementCount == 1);

		const uint8* HeaderStart = (const uint8*)(ChunkStart + TMath::Align4((int32)sizeof(TResfileChunkHeader)));
		const uint8* SceneTileDataStart = HeaderStart + TMath::Align4((int32)sizeof(THeaderSceneTile)) * 1;

		{
			const THeaderSceneTile* Header = (const THeaderSceneTile*)(HeaderStart);
			TSceneTileResourcePtr SceneTile = ti_new TSceneTileResource;
			SceneTile->LevelName = GetString(Header->LevelNameIndex);
			SceneTile->TotalEnvLights = Header->NumEnvLights;
			SceneTile->TotalMeshes = Header->NumMeshes;
			SceneTile->TotalMeshSections = Header->NumMeshSections;
			SceneTile->TotalInstances = Header->NumInstances;
			SceneTile->Position.X = Header->Position.X;
			SceneTile->Position.Y = Header->Position.Y;
			SceneTile->BBox = Header->BBox;
			// Add mesh sections info to scene tile 

			TAssetLibrary* AssetLib = TAssetLibrary::Get();
			// Load reflection captures
			const THeaderEnvLight* EnvLightData = (const THeaderEnvLight*)SceneTileDataStart;
			SceneTile->EnvLights.reserve(Header->NumEnvLights);
			SceneTile->EnvLightInfos.reserve(Header->NumEnvLights);

			for (int32 rc = 0; rc < Header->NumEnvLights; ++rc)
			{
				const THeaderEnvLight& RC = EnvLightData[rc];
				// Load cubemap
				TString Cubemap = GetString(RC.LinkedCubemapIndex);
				TAssetPtr CubemapAsset = AssetLib->LoadAssetAysc(Cubemap);
				SceneTile->EnvLights.push_back(CubemapAsset);

				// Create actor
				TString Name = GetString(RC.NameIndex);

				TSceneTileResource::TEnvLightInfo EnvLightInfo;
				EnvLightInfo.Radius = 9999.f;
				EnvLightInfo.Position = RC.Position;
				SceneTile->EnvLightInfos.push_back(EnvLightInfo);
			}

			// Load assets names
			const int32* AssetsTextures = (const int32*)(SceneTileDataStart + sizeof(THeaderEnvLight) * Header->NumEnvLights);
			const int32* AssetsMaterials = AssetsTextures + Header->NumTextures;
			const int32* AssetsMaterialInstances = AssetsMaterials + Header->NumMaterials;
			const int32* AssetsMeshes = AssetsMaterialInstances + Header->NumMaterialInstances;
			const int32* MeshSections = AssetsMeshes + Header->NumMeshes;
			const int32* MeshInstanceCount = MeshSections + Header->NumMeshes;
			const int32* AssetsInstances = MeshInstanceCount + Header->NumMeshes;

			const THeaderSceneMeshInstance* InstanceData = (const THeaderSceneMeshInstance*)(AssetsInstances);

			// Textures
			for (int32 t = 0; t < Header->NumTextures; ++t)
			{
				TString TextureName = GetString(AssetsTextures[t]);
				AssetLib->LoadAssetAysc(TextureName);
			}
			// Materials
			for (int32 m = 0; m < Header->NumMaterials; ++m)
			{
				TString MaterialName = GetString(AssetsMaterials[m]);
				AssetLib->LoadAssetAysc(MaterialName);
			}
			// Material Instances
			for (int32 mi = 0; mi < Header->NumMaterialInstances; ++mi)
			{
				TString MIName = GetString(AssetsMaterialInstances[mi]);
				AssetLib->LoadAssetAysc(MIName);
			}

			// Load meshes, add it to scene tile node when loading finished.
			SceneTile->Meshes.reserve(Header->NumMeshes);
			for (int32 m = 0; m < Header->NumMeshes; ++m)
			{
				TString MeshName = GetString(AssetsMeshes[m]);
				TAssetPtr MeshAsset = AssetLib->LoadAssetAysc(MeshName);
				SceneTile->Meshes.push_back(MeshAsset);
			}

			// false = all mesh sections in a model share the same instance buffer
			// true = all sections use a separate instance buffer, for GPU cull system
			static const bool ExpandInstanceForEachMeshSections = true;

			// Instances
			TI_ASSERT(Header->NumMeshes > 0 && Header->NumInstances > 0);
			uint32 TotalInstances = 0;
			uint32 TotalMeshSections = 0;
			if (ExpandInstanceForEachMeshSections)
			{
				// Calculate expanded instance count
				for (int32 m = 0; m < Header->NumMeshes; ++m)
				{
					const int32 MeshSectionCount = MeshSections[m];
					const int32 InstanceCount = MeshInstanceCount[m];

					TotalInstances += MeshSectionCount * InstanceCount;
					TotalMeshSections += MeshSectionCount;
				}
				TI_ASSERT(TotalMeshSections == Header->NumMeshSections);
				SceneTile->MeshSectionsCount.reserve(Header->NumMeshes);
			}
			else
			{
				TotalInstances = Header->NumInstances;
				TotalMeshSections = Header->NumMeshes;
			}

			SceneTile->InstanceCountAndOffset.reserve(TotalMeshSections);
			SceneTile->MeshInstanceBuffer = ti_new TInstanceBuffer;
			SceneTile->MeshInstanceBuffer->SetResourceName(Filename + "-TileInstance");
			int8* Data = ti_new int8[TInstanceBuffer::InstanceStride * TotalInstances];

			int32 InstanceOffsetSrc = 0;
			int32 InstanceOffsetDst = 0;
			int32 DataOffset = 0;
			for (int32 m = 0; m < Header->NumMeshes; ++m)
			{
				const int32 InstanceCount = MeshInstanceCount[m];
				
				// Compute instance data for the 1st mesh section
				const int32 InstanceDataStart = DataOffset;
				for (int32 i = 0; i < InstanceCount; ++i)
				{
					const THeaderSceneMeshInstance& Instance = InstanceData[i + InstanceOffsetSrc];

					FFloat4 Transition(Instance.Position.X, Instance.Position.Y, Instance.Position.Z, 0.f);
					FMatrix RotationScaleMat;
					GetInstanceRotationScaleMatrix(RotationScaleMat, Instance.Rotation, Instance.Scale);
#if USE_HALF_FOR_INSTANCE_ROTATION
					FHalf4 RotScaleMat[3];
					MatrixRotationScaleToHalf3(RotationScaleMat, RotScaleMat[0], RotScaleMat[1], RotScaleMat[2]);
#else
					FFloat4 RotScaleMat[3];
					MatrixRotationScaleToFloat3(RotationScaleMat, RotScaleMat[0], RotScaleMat[1], RotScaleMat[2]);
#endif

					memcpy(Data + DataOffset, &Transition, sizeof(FFloat4));
					DataOffset += sizeof(FFloat4);
					memcpy(Data + DataOffset, RotScaleMat, sizeof(RotScaleMat));
					DataOffset += sizeof(RotScaleMat);
				}
				const int32 InstanceDataLength = DataOffset - InstanceDataStart;
				// Save instance offset and count
				SceneTile->InstanceCountAndOffset.push_back(vector2di(InstanceCount, InstanceOffsetDst));
				InstanceOffsetSrc += InstanceCount;
				InstanceOffsetDst += InstanceCount;

				if (ExpandInstanceForEachMeshSections)
				{
					// Copy the same instance data for other mesh sections
					const int32 MeshSectionCount = MeshSections[m];
					TI_ASSERT(MeshSectionCount > 0);

					for (int32 s = 1; s < MeshSectionCount; ++s)
					{
						memcpy(Data + DataOffset, Data + InstanceDataStart, InstanceDataLength);
						DataOffset += InstanceDataLength;
						// Save instance offset and count
						SceneTile->InstanceCountAndOffset.push_back(vector2di(InstanceCount, InstanceOffsetDst));
						InstanceOffsetDst += InstanceCount;
					}
					SceneTile->MeshSectionsCount.push_back(MeshSectionCount);
				}
			}

			SceneTile->MeshInstanceBuffer->SetInstanceStreamData(TInstanceBuffer::InstanceFormat, Data, TotalInstances);
			TI_ASSERT(InstanceOffsetDst == TotalInstances);
			ti_delete[] Data;
			FStats::Stats.InstancesLoaded += TotalInstances;

			OutResources.push_back(SceneTile);
		}
	}
}
