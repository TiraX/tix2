/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_PIXEL_FORMAT
	{
		EPF_A8,
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

		// Compressed formats
		// DXT formats
		EPF_DDS_DXT1,
		EPF_DDS_DXT3,
		EPF_DDS_DXT5,
		EPF_DDS_BC5,

		// ASTC formats 
		EPF_COMPRESSED_ASTC1,
		EPF_COMPRESSED_ASTC2,
		EPF_COMPRESSED_ASTC3,
		EPF_COMPRESSED_ASTC4,

		// ETC format
		EPF_COMPRESSED_ETC,
		
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
}