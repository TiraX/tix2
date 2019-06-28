/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FVTSystem.h"
#include "FVTTaskThread.h"

#if defined (TIX_DEBUG)
//#	define DEBUG_IT_INFO
#endif

namespace tix
{
	const bool FVTSystem::Enabled = true;
	FVTSystem * FVTSystem::VTSystem = nullptr;

	const float FVTSystem::UVInv = (float)PPSize / (float)VTSize;

	FVTSystem::FVTSystem()
		: VTTaskThread(nullptr)
		, VTRegion(VTSize, PPSize)
		, IndirectTextureDirty(false)
	{
		TI_ASSERT(IsRenderThread());
		VTSystem = this;

		// Start task thread
		VTTaskThread = ti_new FVTTaskThread;
		VTTaskThread->Start();

		// Init region data
		RegionData.resize(ITSize * ITSize);
		memset(RegionData.data(), -1, ITSize * ITSize * sizeof(int32));

		// Init render resources
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			PhysicPageResource[i] = FRHI::Get()->CreateRenderResourceTable(PPCount, EHT_SHADER_RESOURCE);
		}
		PhysicPageTextures.resize(PPCount);
		for (int32 i = 0; i < PPCount; ++i)
		{
			PhysicPageTextures[i] = uint32(-1);
		}
		IndirectTextureData = ti_new TImage(EPF_R16F, ITSize, ITSize);
		IndirectTextureDirty = true;

		// Init available locations
		AvailbleLocations.reserve(PPCount);
		FillAvailbleLocations();
	}

	FVTSystem::~FVTSystem()
	{
		TI_ASSERT(IsRenderThread());
		VTSystem = nullptr;
		VTTaskThread->Stop();
		ti_delete VTTaskThread;
		VTTaskThread = nullptr;

		PhysicPageTextures.clear();
		for (int32 i = 0; i < FRHIConfig::FrameBufferNum; ++i)
		{
			PhysicPageResource[i] = nullptr;
		}
		//IndirectTextureData = nullptr;
	}

	uint32 FVTSystem::GetPrimitiveTextureHash(FPrimitivePtr InPrimitive)
	{
		const TVector<TString>& TextureNames = InPrimitive->GetArgumentBuffer()->GetTextureNames();
		TString LongStr;
		for (const auto& S : TextureNames)
		{
			LongStr += S;
		}
		return TCrc::StringHash(LongStr.c_str());
	}

	void FVTSystem::AllocatePositionForPrimitive(FPrimitivePtr InPrimitive)
	{
		if (!Enabled)
			return;

		const TVector<TString>& TextureNames = InPrimitive->GetArgumentBuffer()->GetTextureNames();
		const TVector<vector2di>& TextureSizes = InPrimitive->GetArgumentBuffer()->GetTextureSizes();
		if (TextureSizes.size() == 0)
			return;

		uint32 TexturesHash = GetPrimitiveTextureHash(InPrimitive);
		uint32 RegionIndex = uint32(-1);
		TRegion::TRegionDesc* Region = nullptr;

		THMap<uint32, FRegionInfo>::iterator It = RegionsAllocated.find(TexturesHash);
		if (It != RegionsAllocated.end())
		{
			// Already allocated
			FRegionInfo& Info = It->second;
			Info.Refs++;
			RegionIndex = Info.RegionIndex;
			Region = VTRegion.GetRegionByIndex(RegionIndex);
		}
		else
		{
			// Allocate a new region
			int32 W = TextureSizes[0].X;
			int32 H = TextureSizes[0].Y;

			// Assume all textures are power of 2 sizes
			//VTRegion.GetRegionSizeRequirement(W, H);
			Region = VTRegion.FindAvailbleRegion(W, H, &RegionIndex);
			TI_ASSERT(RegionIndex != uint32(-1));
			FRegionInfo Info;
			Info.RegionIndex = RegionIndex;
			Info.Refs = 1;
			RegionsAllocated[TexturesHash] = Info;

			// Add texture info
			TI_ASSERT(TexturesInVT.find(RegionIndex) == TexturesInVT.end());
			TexturesInVT[RegionIndex] = FTextureInfo(TextureNames[0], vector2du16(TextureSizes[0].X, TextureSizes[0].Y));

			// Mark texture info in this region
			MarkRegion(RegionIndex, Region);
		}

		TI_ASSERT(RegionIndex != uint32(-1));

		int32 x = RegionIndex % ITSize;
		int32 y = RegionIndex / ITSize;
		InPrimitive->SetUVTransform(x * UVInv, y * UVInv, Region->XCount * UVInv, Region->YCount * UVInv);
		InPrimitive->SetVTDebugInfo((float)x, (float)y + 1.f, (float)Region->XCount, (float)Region->YCount);
		//InPrimitive->SetVTDebugInfo(0.f, 1.f, 0.f, 0.f);
	}

	void FVTSystem::MarkRegion(uint32 InRegionIndex, TRegion::TRegionDesc * InRegion)
	{
		int32 Index = InRegionIndex;
		for (int32 y = 0 ; y < InRegion->YCount ; ++ y)
		{
			for (int32 x = 0 ; x < InRegion->XCount ; ++ x)
			{
				TI_ASSERT(RegionData[Index + x] < 0);
				RegionData[Index + x] = (int32)InRegionIndex;
			}
			Index += ITSize;
		}
	}

	void FVTSystem::RemovePositionForPrimitive(FPrimitivePtr InPrimitive)
	{
		if (!Enabled)
			return;

		TI_ASSERT(0);
		uint32 TexturesHash = GetPrimitiveTextureHash(InPrimitive);
		THMap<uint32, FRegionInfo>::iterator It = RegionsAllocated.find(TexturesHash);
		TI_ASSERT(It != RegionsAllocated.end());
		FRegionInfo& Info = It->second;
		--Info.Refs;
		TI_ASSERT(Info.Refs >= 0);
		if (Info.Refs == 0)
		{
			// Return the region back.
			TI_ASSERT(0);

			// delete RegionsAllocated
			// remove TexturesInVT
			// clear RegionData
		}
	}

	void FVTSystem::GetPageInfoByPosition(const vector2di& InPosition, FPageInfo& OutInfo)
	{
		int32 PageX = InPosition.X / PPSize;
		int32 PageY = InPosition.Y / PPSize;
		TI_ASSERT(PageX >= 0 && PageY >= 0);
		uint32 PageIndex = PageY * ITSize + PageX;
#ifdef DEBUG_IT_INFO
		if (RegionData[PageIndex] == -1)
		{
			int32 out = false;
			if (out)
			{
				OutputDebugInfo();
			}
			FPageInfo PageInfo;
			return PageInfo;
		}
#endif
		TI_ASSERT(RegionData[PageIndex] != -1);
		int32 RegionIndex = RegionData[PageIndex];
		TI_ASSERT(RegionIndex != -1);
		TI_ASSERT(TexturesInVT.find(RegionIndex) != TexturesInVT.end());

		int32 RegionCellStartX = RegionIndex % ITSize;
		int32 RegionCellStartY = RegionIndex / ITSize;

		FTextureInfo& TextureInfo = TexturesInVT[RegionIndex];
		OutInfo.TextureName = TextureInfo.TextureName;
		OutInfo.TextureSize = TextureInfo.TextureSize;
		OutInfo.PageStart.X = PageX - RegionCellStartX;
		OutInfo.PageStart.Y = PageY - RegionCellStartY;
		OutInfo.PhysicPage.X = PageX;
		OutInfo.PhysicPage.Y = PageY;
		OutInfo.PageIndex = PageIndex;
	}

	void FVTSystem::OutputDebugInfo()
	{
#ifdef DEBUG_IT_INFO
		// Indirect texture 
		TStringStream ss;
		THMap<int32, int32> Images;
		for (int32 x = 0; x < ITSize; ++x)
		{
			ss << x + 1 << ", ";
		}
		ss << "\n";
		for (int32 y = 0 ; y < ITSize ; ++ y)
		{
			for (int32 x = 0 ; x < ITSize ; ++ x)
			{
				int32 PageIndex = y * ITSize + x;
				int32 RegionIndex = RegionData[PageIndex];
				if (RegionIndex == -1)
				{
					ss << "-1, ";
				}
				else
				{
					ss << RegionIndex << ", ";
					Images[RegionIndex] = 1;
				} 
			}
			ss << "\n";
		}

		ss << "Images : , " << Images.size() << " \n";

		TFile RegionDataFile;
		RegionDataFile.Open("region_data.csv", EFA_CREATEWRITE);
		TString csvString = ss.str();
		RegionDataFile.Write(csvString.c_str(), (int32)csvString.size());
#endif
	}

	void FVTSystem::FillAvailbleLocations()
	{
		AvailbleLocations.clear();
		for (uint32 i = 0 ; i < PPCount ; ++ i)
		{
			AvailbleLocations[i] = 0;
		}
	}

	void FVTSystem::UpdatePhysicPage_RenderThread(FPhysicPageTexture& PhysicPageTexture)
	{
		TI_ASSERT(IsRenderThread());

		// Create texture render thread resource
		FTexturePtr TextureResource;
		if (PhysicPageTexture.TextureResource == nullptr)
		{
			TI_ASSERT(PhysicPageTexture.Texture->TextureResource != nullptr);
			FRHI::Get()->UpdateHardwareResourceTexture(PhysicPageTexture.Texture->TextureResource, PhysicPageTexture.Texture);
			TextureResource = PhysicPageTexture.Texture->TextureResource;
		}
		else
		{
			TextureResource = PhysicPageTexture.TextureResource;
			TI_ASSERT(TextureResource->HasTextureFlag(ETF_RENDER_RESOURCE_UPDATED));
		}

		uint32 PageIndex = PhysicPageTexture.PhysicPagePosition.Y * ITSize + PhysicPageTexture.PhysicPagePosition.X;
		// Check which textures should be add to texture array
		if (PhysicPagesMap.find(PageIndex) == PhysicPagesMap.end())
		{
			// Not in PhysicPageTextures, remember it, add it later in PrepareVTIndirectTexture()
			TI_ASSERT(NewPages.find(PageIndex) == NewPages.end());
			NewPages[PageIndex] = TextureResource;
		}
		else
		{
			// Already in PhysicPageTextures, remove this location from AvailbleLocations
			TI_ASSERT(AvailbleLocations.find(PhysicPagesMap[PageIndex]) != AvailbleLocations.end());
			AvailbleLocations.erase(PhysicPagesMap[PageIndex]);
		}
		
		// Pages updated in one frame can not exceed PPCount(1024)
		TI_ASSERT(NewPages.size() < PPCount);

		IndirectTextureDirty = true;
	}

	void FVTSystem::PrepareVTIndirectTexture()
	{
		TI_ASSERT(IsRenderThread());
		if (IndirectTextureDirty)
		{
			FRenderResourceTablePtr VTTable = PhysicPageResource[FRHI::Get()->GetCurrentEncodingFrameIndex()];

			// Fill new pages into Physic Page Array
			SColorf Color;
			for (auto& P : NewPages)
			{
				uint32 PageIndex = P.first;
				FTexturePtr TextureResource = P.second;

				// Page position
				int32 PageX = PageIndex % ITSize;
				int32 PageY = PageIndex / ITSize;

				// Location in PhysicPageTextures
				TI_ASSERT(AvailbleLocations.size() > 0);
				uint32 Location = AvailbleLocations.begin()->first;

				// Remove old texture info from PhysicPagesMap
				if (PhysicPageTextures[Location] != uint32(-1))
				{
					TI_ASSERT(PhysicPagesMap.find(PhysicPageTextures[Location]) != PhysicPagesMap.end());
					PhysicPagesMap.erase(PhysicPageTextures[Location]);
				}

				// Put texture in this array
				VTTable->PutTextureInTable(TextureResource, Location);
				PhysicPageTextures[Location] = PageIndex;

				// Remember this page is in array
				PhysicPagesMap[PageIndex] = Location;

				// Remove this avaible location
				AvailbleLocations.erase(AvailbleLocations.begin());

				// Remember this in indirect texture
				Color.R = float(Location);
				IndirectTextureData->SetPixel(PageX, PageY, Color);
			}

			TI_TODO("Create texture from TImage.");

			NewPages.clear();
			FillAvailbleLocations();

			IndirectTextureDirty = false;
		}
	}
}