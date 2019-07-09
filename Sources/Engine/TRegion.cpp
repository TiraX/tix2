/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TRegion.h"

namespace tix
{
	TRegion::TRegion()
		: RegionSize(0)
		, CellSize(0)
	{
	}
	TRegion::TRegion(int32 InRegionSize, int32 InCellSize)
	{
		Reset(InRegionSize, InCellSize);
	}

	TRegion::~TRegion()
	{
	}

#define DEBUG_REGIONS	0

#if DEBUG_REGIONS
#define DEBUG_REGION_PRINT	_LOG
#define DEBUG_VERIFY_INTEGRITY() verify_integrity();
#else
#define DEBUG_REGION_PRINT
#define DEBUG_VERIFY_INTEGRITY()
#endif

	TRegion::TRegionDesc* TRegion::FindAvailbleRegion(int32 Width, int32 Height, uint32 * OutRegionIndex)
	{
		DEBUG_VERIFY_INTEGRITY();

		// extend 1 pixel for safe rendering
		int32 XCount = (Width + 1) / CellSize;
		int32 YCount = (Height + 1) / CellSize;
		int32 Match = -1;

		TI_ASSERT(XCount > 0 && YCount > 0);

		DEBUG_REGION_PRINT(Log, "find_available_region %dx%d (%dx%d)\n", Width, Height, XCount, YCount);

		for (uint32 i = 0; i < AvailableRegions.size(); i++)
		{
			const TRegionDesc& AvailableRegionDesc = Regions[AvailableRegions[i]];
			DEBUG_REGION_PRINT(Log, "\ttest TRegionDesc %d %dx%d\n", i, AvailableRegionDesc.XCount, AvailableRegionDesc.YCount);
			if (AvailableRegionDesc.XCount >= XCount && AvailableRegionDesc.YCount >= YCount)
			{
				// better than current match?
				if (Match == -1)
				{
					Match = i;
				}
				else
				{
					const TRegionDesc& MatchRegion = Regions[AvailableRegions[Match]];
					if (AvailableRegionDesc.XCount < MatchRegion.XCount || AvailableRegionDesc.YCount < MatchRegion.YCount)
					{
						Match = i;
					}
				}
			}
		}

		DEBUG_REGION_PRINT(Log, "\tmatch=%d\n", Match);

		if (Match != -1)
		{
			uint32 MatchRegionId = AvailableRegions[Match];
			const TRegionDesc& MatchRegion = Regions[MatchRegionId];

			// no more available
			TVector<uint32>::iterator it = AvailableRegions.begin() + Match;
			AvailableRegions.erase(it);

			// need to subdivide TRegionDesc?
			if (MatchRegion.XCount > XCount || MatchRegion.YCount > YCount)
				SubDivideRegion(MatchRegionId, XCount, YCount);

			DEBUG_VERIFY_INTEGRITY();
			if (OutRegionIndex != nullptr)
			{
				*OutRegionIndex = MatchRegionId;
			}
			return &Regions[MatchRegionId];
		}

		return nullptr;
	}

	TRegion::TRegionDesc* TRegion::GetRegionByIndex(uint32 RegionIndex)
	{
		return &Regions[RegionIndex];
	}

	void TRegion::SubDivideRegion(uint32 MatchRegionId, int32 NewXCount, int32 NewYCount)
	{
		DEBUG_VERIFY_INTEGRITY();

		TRegionDesc& R = Regions[MatchRegionId];

		const int32 XCount = R.XCount;
		const int32 YCount = R.YCount;
		const int32 Pitch = RegionSize / CellSize;
		//m_bitmap->get_width() / CellSize;
		const int32 Offset = (int32)(MatchRegionId);

		TI_ASSERT(NewXCount > 0 && NewYCount > 0);

		DEBUG_REGION_PRINT(Log, "subdivide TRegionDesc %dx%d -> %dx%d\n", XCount, YCount, NewXCount, NewYCount);

		// right residue?
		if (XCount - NewXCount > 0)
		{
			TRegionDesc* Right = &Regions[Offset + NewXCount];
			Right->XCount = XCount - NewXCount;
			Right->YCount = NewYCount;
			DEBUG_REGION_PRINT(Log, "\tright %dx%d\n", Right->XCount, Right->YCount);
			AvailableRegions.push_back(Offset + NewXCount);

			DEBUG_VERIFY_INTEGRITY();
		}

		// bottom residue?
		if (YCount - NewYCount > 0)
		{
			TRegionDesc* Bottom = &Regions[Offset + Pitch * NewYCount];
			Bottom->XCount = XCount;
			Bottom->YCount = YCount - NewYCount;
			DEBUG_REGION_PRINT(Log, "\tbottom %dx%d\n", Bottom->XCount, Bottom->YCount);
			AvailableRegions.push_back(Offset + Pitch * NewYCount);

			DEBUG_VERIFY_INTEGRITY();
		}

		// resized TRegionDesc
		R.XCount = NewXCount;
		R.YCount = NewYCount;

		DEBUG_VERIFY_INTEGRITY();
	}

	void TRegion::GetRegionSizeRequirement(int32& Width, int32& Height)
	{
		// multiple of CellSize
		int32 XCount = Width / CellSize;
		Width = (Width % CellSize > 0) ? (XCount + 1) * CellSize : XCount * CellSize;
		Width = ti_max(Width, CellSize);

		int32 YCount = Height / CellSize;
		Height = (Height % CellSize > 0) ? (YCount + 1) * CellSize : YCount * CellSize;
		Height = ti_max(Height, CellSize);
	}

	void TRegion::Reset(int32 InRegionSize, int32 InCellSize)
	{
		RegionSize = InRegionSize;
		CellSize = InCellSize;

		AvailableRegions.clear();

		// init 1st region with whole surface
		TRegionDesc R;
		R.XCount = RegionSize / CellSize;
		R.YCount = RegionSize / CellSize;
		
		// allocate region infos
		Regions.resize(R.XCount * R.YCount);

		Regions[0] = R;
		AvailableRegions.push_back(0);
	}
}
