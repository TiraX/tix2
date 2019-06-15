/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FVTSystem.h"
#include "FVTTaskThread.h"

#if defined (TIX_DEBUG)
#	define DEBUG_IT_INFO
#endif

namespace tix
{
	const bool FVTSystem::Enabled = true;
	FVTSystem * FVTSystem::VTSystem = nullptr;

	const float FVTSystem::UVInv = (float)PPSize / (float)VTSize;

	FVTSystem::FVTSystem()
		: VTRegion(VTSize, PPSize)
	{
		VTSystem = this;

		// Start task thread
		VTTaskThread = ti_new FVTTaskThread;
		VTTaskThread->Start();

		// Init region data
		RegionData.resize(ITSize * ITSize);
		memset(RegionData.data(), -1, ITSize * ITSize * sizeof(int32));
	}

	FVTSystem::~FVTSystem()
	{
		VTSystem = nullptr;
		VTTaskThread->Stop();
		ti_delete VTTaskThread;
		VTTaskThread = nullptr;
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
		TRegion::TRegionDesc * Region = nullptr;

		THMap<uint32, FRegionInfo>::iterator It = RegionsAllocated.find(TexturesHash);
		if (It != RegionsAllocated.end())
		{
			// Already allocated
			FRegionInfo& Info = It->second;
			Info.Refs++;
			Region = Info.Desc;
		}
		else
		{
			// Allocate a new region
			int32 W = TextureSizes[0].X;
			int32 H = TextureSizes[0].Y;

			// Assume all textures are power of 2 sizes
			//VTRegion.GetRegionSizeRequirement(W, H);
			uint32 RegionIndex = uint32(-1);
			Region = VTRegion.FindAvailbleRegion(W, H, &RegionIndex);
			TI_ASSERT(RegionIndex < 0x7fffffff);
			FRegionInfo Info;
			Info.Desc = Region;
			Info.Refs = 1;
			RegionsAllocated[TexturesHash] = Info;

			// Add texture info
			TI_ASSERT(TexturesInVT.find(RegionIndex) == TexturesInVT.end());
			TexturesInVT[RegionIndex] = FTextureInfo(TextureNames[0], vector2du16(TextureSizes[0].X, TextureSizes[0].Y));

			// Mark texture info in this region
			MarkRegion(RegionIndex, Region);
			int32 x = RegionIndex % ITSize;
			int32 y = RegionIndex / ITSize;
			InPrimitive->SetVTDebugInfo(x, y, W, H);
		}

		TI_ASSERT(Region != nullptr);

		InPrimitive->SetUVTransform(Region->XCount * UVInv, Region->YCount * UVInv, UVInv, UVInv);
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
#ifdef DEBUG_IT_INFO
		static int32 counter = 0;
		int32 x = Index % ITSize;
		int32 y = Index / ITSize;
		_LOG(Log, "%d, Mark region %d, %d, [%d, %d]\n", counter ++, x, y, InRegion->XCount, InRegion->YCount);
#endif
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

	FVTSystem::FPageInfo FVTSystem::GetPageInfoByPosition(const vector2di& InPosition)
	{
		FPageInfo PageInfo;
		int32 PageX = InPosition.X / PPSize;
		int32 PageY = InPosition.Y / PPSize;
		int32 PageIndex = PageY * ITSize + PageX;
#ifdef DEBUG_IT_INFO
		if (RegionData[PageIndex] == -1)
		{
			int32 out = false;
			if (out)
			{
				OutputDebugInfo();
			}
			FPageInfo PageInfo;
			PageInfo.RegionData = 0x80000000;
			return PageInfo;
		}
#endif
		TI_ASSERT(RegionData[PageIndex] != -1);
		int32 RegionIndex = RegionData[PageIndex] & 0x7fffffff;
		TI_ASSERT(RegionIndex != -1);
		TI_ASSERT(TexturesInVT.find(RegionIndex) != TexturesInVT.end());

		int32 RegionCellStartX = RegionIndex % ITSize;
		int32 RegionCellStartY = RegionIndex / ITSize;

		FTextureInfo& TextureInfo = TexturesInVT[RegionIndex];
		PageInfo.TextureName = TextureInfo.TextureName;
		PageInfo.TextureSize = TextureInfo.TextureSize;
		PageInfo.PageStart.X = PageX - RegionCellStartX;
		PageInfo.PageStart.Y = PageY - RegionCellStartY;
		PageInfo.RegionData = uint32(RegionData[PageIndex]);

		return PageInfo;
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
					int32 Loaded = (RegionIndex & 0x80000000) >> 31;
					RegionIndex = RegionIndex & 0x7fffffff;
					ss << Loaded << "/" << RegionIndex << ", ";
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
}