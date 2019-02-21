/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_PIXEL_FORMAT
	{
		EPF_A8,
		EPF_RGB8,	// As TGA need it
		EPF_BGR8,	// As TGA need it
		EPF_RGBA8,
		EPF_RGBA8_SRGB,
		EPF_BGRA8,
		EPF_BGRA8_SRGB,

		// Float formats
		EPF_R16F,
		EPF_RG16F,
		EPF_RGBA16F,
		EPF_R32F,
		EPF_RG32F,
		EPF_RGB32F,
		EPF_RGBA32F,

		// Depth formats
		EPF_DEPTH16,
		EPF_DEPTH32,
		EPF_DEPTH24_STENCIL8,
		EPF_STENCIL8,

		// Compressed formats
		// DXT formats
		EPF_DDS_DXT1,
		EPF_DDS_DXT1_SRGB,
		EPF_DDS_DXT3,
		EPF_DDS_DXT3_SRGB,
		EPF_DDS_DXT5,
		EPF_DDS_DXT5_SRGB,
		EPF_DDS_BC5,

		// ASTC formats 
		EPF_ASTC4x4,
		EPF_ASTC4x4_SRGB,
		EPF_ASTC6x6,
		EPF_ASTC6x6_SRGB,
		EPF_ASTC8x8,
		EPF_ASTC8x8_SRGB,
		
		EPF_UNKNOWN,
		EPF_COUNT
	};

	enum E_PIXEL_COMPONENT
	{
		EPC_RED,
		EPC_GREEN,
		EPC_BLUE,
		EPC_ALPHA,

		EPC_COUNT
	};

	inline E_PIXEL_FORMAT GetSRGBFormat(E_PIXEL_FORMAT SrcFormat)
	{
		switch (SrcFormat)
		{
		case EPF_RGBA8:
		case EPF_BGRA8:
		case EPF_ASTC4x4:
		case EPF_ASTC6x6:
		case EPF_ASTC8x8:
		case EPF_DDS_DXT1:
		case EPF_DDS_DXT3:
		case EPF_DDS_DXT5:
			return (E_PIXEL_FORMAT)(SrcFormat + 1);
		case EPF_RGBA8_SRGB:
		case EPF_BGRA8_SRGB:
		case EPF_ASTC4x4_SRGB:
		case EPF_ASTC6x6_SRGB:
		case EPF_ASTC8x8_SRGB:
		case EPF_DDS_DXT1_SRGB:
		case EPF_DDS_DXT3_SRGB:
		case EPF_DDS_DXT5_SRGB:
			return SrcFormat;
		}
		TI_ASSERT(0);
		return EPF_UNKNOWN;
	}
}