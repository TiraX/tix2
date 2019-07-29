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
	TVector<int32> FVTSystem::VTMipsOffset;

	const float FVTSystem::UVInv = (float)PPSize / (float)VTSize;

	FVTSystem::FVTSystem()
		: VTAnalysisThread(nullptr)
		, VTRegion(VTSize, PPSize)
	{
		TI_ASSERT(IsRenderThread());
		VTSystem = this;

		// Init mip offset array
		TI_ASSERT(VTMipsOffset.size() == 0);
		int32 TotalPages = 0;
		int32 MipSize = VTSize;
		while (MipSize >= PPSize)
		{
			int32 Rows = MipSize / PPSize;
			TotalPages += Rows * Rows;
			VTMipsOffset.push_back(TotalPages);
			MipSize /= 2;
		}

		// Start task thread
		VTAnalysisThread = ti_new FVTAnalysisThread;
		VTAnalysisThread->Start();

		// Init region data
		RegionData.resize(ITSize * ITSize);
		memset(RegionData.data(), -1, ITSize * ITSize * sizeof(int32));

		// Init render resources
		VTResource = FRHI::Get()->CreateRenderResourceTable(PPCount, EHT_SHADER_RESOURCE);

		// Create indirect texture
		IndirectTextureData = ti_new TImage(EPF_RGBA8, ITSize, ITSize);
		IndirectTextureData->AllocEmptyMipmaps();
		TTextureDesc Desc;
		Desc.Type = ETT_TEXTURE_2D;
		Desc.Format = EPF_RGBA8;
		Desc.Width = ITSize;
		Desc.Height = ITSize;
		Desc.AddressMode = ETC_CLAMP_TO_EDGE;
		Desc.SRGB = 0;
		Desc.Mips = (uint32)VTMipsOffset.size();
		IndirectTexture = FRHI::Get()->CreateTexture(Desc);
		FRHI::Get()->UpdateHardwareResourceTexture(IndirectTexture);

		// Create physic texture atlas
		Desc.Format = PageFormat;
		Desc.Width = PhysicAtlasSize * PPSize;
		Desc.Height = PhysicAtlasSize * PPSize;
		Desc.SRGB = 1;
		PhysicPageAtlas = FRHI::Get()->CreateTexture(Desc);
		FRHI::Get()->UpdateHardwareResourceTexture(PhysicPageAtlas);
		VTResource->PutTextureInTable(IndirectTexture, 0);
		VTResource->PutTextureInTable(PhysicPageAtlas, 1);
	}

	FVTSystem::~FVTSystem()
	{
		TI_ASSERT(IsRenderThread());
		VTSystem = nullptr;
		VTAnalysisThread->Stop();
		ti_delete VTAnalysisThread;
		VTAnalysisThread = nullptr;

		IndirectTexture = nullptr;
		PhysicPageAtlas = nullptr;
		VTResource = nullptr;
		IndirectTextureData = nullptr;
	}

#if VT_PRELOADED_REGIONS
	void FVTSystem::InitLoadedTextureRegion(const TString& TextureName, const vector4di& Region)
	{
		LoadedRegionInfos[TextureName] = Region;
	}
#endif

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

#if !(VT_PRELOADED_REGIONS)
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
			MarkRegion(RegionIndex, Region->XCount, Region->YCount);
		}

		TI_ASSERT(RegionIndex != uint32(-1));

		int32 X = RegionIndex % ITSize;
		int32 Y = RegionIndex / ITSize;
		int32 W = Region->XCount;
		int32 H = Region->YCount;
#else
		TI_ASSERT(LoadedRegionInfos.find(TextureNames[0]) != LoadedRegionInfos.end());
		const vector4di& LoadedRegion = LoadedRegionInfos[TextureNames[0]];
		int32 X = LoadedRegion.X;
		int32 Y = LoadedRegion.Y;
		int32 W = LoadedRegion.Z;
		int32 H = LoadedRegion.W;
		TI_ASSERT(W >= 0 && W <= 8 && H >= 0 && H <= 8);
		TI_TODO("RegionData and TexturesInVT is debug info in VT_PRELOADED_REGIONS mode. remove them.");
		int32 RegionIndex = Y * ITSize + X;
		MarkRegion(RegionIndex, W, H);
		// Add texture info
		if (TexturesInVT.find(RegionIndex) == TexturesInVT.end())
		{
			TexturesInVT[RegionIndex] = FTextureInfo(TextureNames[0], vector2du16(TextureSizes[0].X, TextureSizes[0].Y));
		}
#endif

		InPrimitive->SetUVTransform(X * UVInv, Y * UVInv, W * UVInv, H * UVInv);
		InPrimitive->SetVTDebugInfo((float)X, (float)Y + 1.f, (float)W, (float)H);
		//InPrimitive->SetVTDebugInfo(0.f, 1.f, 0.f, 0.f);
	}

	void FVTSystem::MarkRegion(uint32 InRegionIndex, int32 W, int32 H)
	{
		int32 Index = InRegionIndex;
		for (int32 y = 0 ; y < H ; ++ y)
		{
			for (int32 x = 0 ; x < W ; ++ x)
			{
				TI_ASSERT(RegionData[Index + x] < 0 || RegionData[Index + x] == InRegionIndex);
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

	void FVTSystem::GetPageLoadInfoByPageIndex(uint32 PageIndex, FPageLoadInfo& OutInfo)
	{
		int32 PageX = PageIndex % FVTSystem::ITSize;
		int32 PageY = PageIndex / FVTSystem::ITSize;
		TI_ASSERT(PageX >= 0 && PageY >= 0 && PageY < FVTSystem::ITSize);
#ifdef DEBUG_IT_INFO
		if (RegionData[PageIndex] == -1)
		{
			int32 out = false;
			if (out)
			{
				OutputDebugInfo();
			}
			FPageLoadInfo PageInfo;
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
		//OutInfo.TextureSize = TextureInfo.TextureSize;
		OutInfo.PageStart.X = PageX - RegionCellStartX;
		OutInfo.PageStart.Y = PageY - RegionCellStartY;
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

	void FVTSystem::AddVTPageData(uint32 PageIndex, uint32 MipLevel, uint32 AtlasLocation, TTexturePtr TextureData)
	{
		TI_ASSERT(IsRenderThread());
		LoadedPages.push_back(FPageLoadResult(PageIndex, MipLevel, AtlasLocation, TextureData));
	}

	void FVTSystem::PrepareVTIndirectTexture()
	{
		TI_ASSERT(IsRenderThread());

		SColor IndirectData;
		for (auto& Page : LoadedPages)
		{
			uint32 MipLevel = Page.MipLevel;
			int32 MipITSize = ITSize >> MipLevel;

			int32 PageX = Page.PageIndex % MipITSize;
			int32 PageY = Page.PageIndex / MipITSize;
			TI_ASSERT(PageX < MipITSize && PageY < MipITSize);

			int32 AtlasX = Page.AtlasLocation % PhysicAtlasSize;
			int32 AtlasY = Page.AtlasLocation / PhysicAtlasSize;
			TI_ASSERT(AtlasY < PhysicAtlasSize);

			// Update indirect texture
			IndirectData.R = AtlasX;
			IndirectData.G = AtlasY;
			IndirectTextureData->SetPixel(PageX, PageY, IndirectData, MipLevel);

			// Update Physic page atlas regions
			int32 RegionStartX = AtlasX * PPSize;
			int32 RegionStartY = AtlasY * PPSize;
			recti TargetRegion(RegionStartX, RegionStartY, RegionStartX + PPSize, RegionStartY + PPSize);
			TTexturePtr PhysicPage = Page.TextureData;
			TI_ASSERT(PhysicPage->TextureResource == nullptr);
			PhysicPage->TextureResource = FRHI::Get()->CreateTexture(PhysicPage->GetDesc());
			FRHI::Get()->UpdateHardwareResourceTexture(PhysicPage->TextureResource, PhysicPage);
			FRHI::Get()->UpdateHardwareResourceTextureRegion(PhysicPageAtlas, PhysicPage->TextureResource, TargetRegion);
		}
		FRHI::Get()->UpdateHardwareResourceTexture(IndirectTexture, IndirectTextureData);
		LoadedPages.clear();

	}
}