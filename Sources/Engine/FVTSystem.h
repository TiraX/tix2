/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "TRegion.h"

namespace tix
{
	// Refs:
	// Adaptive Virtual Texture Rendering in Far Cry 4
	class FVTAnalysisThread;
	class FVTSystem
	{
	public:
		// Virtual texture size
		static const int32 VTSize = 256 * 1024;
		// Physical page size
		static const int32 PPSize = 256;
		// Indirection texture size
		static const int32 ITSize = VTSize / PPSize;

		// Physical page count
		static const int32 PPCount = 32 * 32;

		static const float UVInv;

		static TI_API bool IsEnabled()
		{
			return Enabled;
		}
		static TI_API FVTSystem * Get()
		{
			return VTSystem;
		}

		FVTSystem();
		~FVTSystem();

		FVTAnalysisThread * GetAnalysisThread()
		{
			return VTAnalysisThread;
		}

		void AllocatePositionForPrimitive(FPrimitivePtr InPrimitive);
		void RemovePositionForPrimitive(FPrimitivePtr InPrimitive);

		void UpdateLoadedPages(const TVector<uint32>& PageIndex, const TVector<FTexturePtr>& TextureResource, const TVector<TTexturePtr>& TextureData);
		TI_API void PrepareVTIndirectTexture();

		struct FPageLoadInfo
		{
			TString TextureName;
			vector2du16 TextureSize;	// No use of this Size, can be removed.
			vector2du16 PageStart;
			uint32 PageIndex;
			FTexturePtr TargetTexture;
			
			FPageLoadInfo()
				: PageIndex(uint32(-1))
			{}

			bool operator < (const FPageLoadInfo& Other) const
			{
				if (TextureName != Other.TextureName)
				{
					return TextureName < Other.TextureName;
				}
				if (PageStart.Y != Other.PageStart.Y)
				{
					return PageStart.Y < Other.PageStart.Y;
				}
				if (PageStart.X != Other.PageStart.X)
				{
					return PageStart.X < Other.PageStart.X;
				}
				return true;
			}
		};
		void GetPageLoadInfoByPosition(const vector2di& InPosition, FPageLoadInfo& OutInfo);

		struct FPhyPageInfo
		{
			FTexturePtr Texture;
			uint32 PageIndex;

			FPhyPageInfo()
				: PageIndex(uint32(-1))
			{}
		};

		void OutputDebugInfo();

	private:
		static uint32 GetPrimitiveTextureHash(FPrimitivePtr InPrimitive);
		void MarkRegion(uint32 InRegionIndex, TRegion::TRegionDesc * InRegion);
		void InsertPhysicPage(const FPhyPageInfo& PhysicPageTexture, THMap<uint32, FTexturePtr>& NewPages, THMap<uint32, uint32>& UsedLocations);
		int32 FindAvailbleLocation(const THMap<uint32, uint32>& UsedLocations);

	private:
		static const bool Enabled;
		static FVTSystem * VTSystem;

		FVTAnalysisThread * VTAnalysisThread;

		TRegion VTRegion;
		struct FRegionInfo
		{
			uint32 RegionIndex;
			int32 Refs;
			FRegionInfo()
				: RegionIndex(uint32(-1))
				, Refs(0)
			{}
		};
		// Texture already in VT, Refs means how many primitives use this texture
		THMap<uint32, FRegionInfo> RegionsAllocated;

		// Data index link to primitive, first 1 bit mark as loaded flag
		TVector<int32> RegionData;

		struct FTextureInfo
		{
			TString TextureName;
			vector2du16 TextureSize;

			FTextureInfo()
			{}

			FTextureInfo(const TString& InName, const vector2du16& InSize)
				: TextureName(InName)
				, TextureSize(InSize)
			{}
		};
		// Texture loaded, Key is offset in RegionData
		THMap<uint32, FTextureInfo> TexturesInVT;


		// Physic texture page array, stores the page index in virtual texture
		TVector<uint32> PhysicPageTextures;

		// Physic page's location in PhysicPageTextures
		// Key is the page index in virtual texture, Value is the index in Physic pages array
		THMap<uint32, uint32> PhysicPagesMap;

		// Next availble location;
		int32 SearchedLocation;
		
		// Physic pages array render resource, slot 0 always be the indirect texture
		FRenderResourceTablePtr PhysicPageResource;

		// Indirect texture data
		TImagePtr IndirectTextureData;

		// Indirect texture
		FTexturePtr IndirectTexture;
	};
}
