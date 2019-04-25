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
	class FVTSystem
	{
	public:
		// Virtual texture size
		static const int32 VTSize = 512 * 1024;
		// Indirection texture size
		static const int32 ITSize = 2 * 1024;
		// Physical page size
		static const int32 PPSize = 256;

		static const float UVInv;

		static bool IsEnabled()
		{
			return Enabled;
		}
		static FVTSystem * Get()
		{
			return VTSystem;
		}

		FVTSystem();
		~FVTSystem();

		void AllocatePositionForPrimitive(FPrimitivePtr InPrimitive);
		void RemovePositionForPrimitive(FPrimitivePtr InPrimitive);

	private:
		static uint32 GetPrimitiveTextureHash(FPrimitivePtr InPrimitive);

	private:
		static const bool Enabled;
		static FVTSystem * VTSystem;

		TRegion VTRegion;
		struct FRegionInfo
		{
			TRegion::TRegionDesc* Desc;
			int32 Refs;
			FRegionInfo()
				: Desc(nullptr)
				, Refs(0)
			{}
		};
		THMap<uint32, FRegionInfo> RegionsAllocated;

		FPipelinePtr PipelineUVCheck;
	};
}
