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
	class FVTTaskThread;
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

		TI_API FVTTaskThread * GetVTTaskThread()
		{
			return VTTaskThread;
		}

		void AllocatePositionForPrimitive(FPrimitivePtr InPrimitive);
		void RemovePositionForPrimitive(FPrimitivePtr InPrimitive);

		struct FPhysicPageTexture
		{
			TTexturePtr Texture;
			vector2du16 PhysicPagePosition;
		};
		void UpdatePhysicPage_RenderThread(FPhysicPageTexture& PhysicPageTexture);
		void PrepareVTIndirectTexture();

		struct FPageInfo
		{
			TString TextureName;
			vector2du16 TextureSize;
			vector2du16 PageStart;
			vector2du16 PhysicPage;
			uint32 PageIndex;

			bool operator < (const FPageInfo& Other) const
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
		void GetPageInfoByPosition(const vector2di& InPosition, FPageInfo& OutInfo);
		void MarkPageAsLoaded(uint32 RegionIndex, bool Loaded);

		void OutputDebugInfo();

	private:
		static uint32 GetPrimitiveTextureHash(FPrimitivePtr InPrimitive);
		void MarkRegion(uint32 InRegionIndex, TRegion::TRegionDesc * InRegion);

	private:
		static const bool Enabled;
		static FVTSystem * VTSystem;

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

		TVector<FTexturePtr> PhysicPageTextures;
		FRenderResourceTablePtr PhysicPageResource;
		bool IndirectTextureDirty;

		FVTTaskThread * VTTaskThread;
	};
}
