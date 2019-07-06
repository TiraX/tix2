/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResHelper.h"
#include "ResTextureHelper.h"
#include "dds.h"
#include "TImage.h"
#include "PlatformUtils.h"
#include <sstream>
#include "ispc_texcomp.h"

namespace tix
{
	//--------------------------------------------------------------------------------------
#define ISBITMASK( r,g,b,a ) ( ddpf.RBitMask == r && ddpf.GBitMask == g && ddpf.BBitMask == b && ddpf.ABitMask == a )

	static DXGI_FORMAT GetDXGIFormat(const DDS_PIXELFORMAT& ddpf)
	{
		if (ddpf.flags & DDS_RGB)
		{
			// Note that sRGB formats are written using the "DX10" extended header

			switch (ddpf.RGBBitCount)
			{
			case 32:
				if (ISBITMASK(0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000))
				{
					return DXGI_FORMAT_R8G8B8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000))
				{
					return DXGI_FORMAT_B8G8R8A8_UNORM;
				}

				if (ISBITMASK(0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000))
				{
					return DXGI_FORMAT_B8G8R8X8_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000000ff,0x0000ff00,0x00ff0000,0x00000000) aka D3DFMT_X8B8G8R8

				// Note that many common DDS reader/writers (including D3DX) swap the
				// the RED/BLUE masks for 10:10:10:2 formats. We assumme
				// below that the 'backwards' header mask is being used since it is most
				// likely written by D3DX. The more robust solution is to use the 'DX10'
				// header extension and specify the DXGI_FORMAT_R10G10B10A2_UNORM format directly

				// For 'correct' writers, this should be 0x000003ff,0x000ffc00,0x3ff00000 for RGB data
				if (ISBITMASK(0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000))
				{
					return DXGI_FORMAT_R10G10B10A2_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x000003ff,0x000ffc00,0x3ff00000,0xc0000000) aka D3DFMT_A2R10G10B10

				if (ISBITMASK(0x0000ffff, 0xffff0000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R16G16_UNORM;
				}

				if (ISBITMASK(0xffffffff, 0x00000000, 0x00000000, 0x00000000))
				{
					// Only 32-bit color channel format in D3D9 was R32F
					return DXGI_FORMAT_R32_FLOAT; // D3DX writes this out as a FourCC of 114
				}
				break;

			case 24:
				// No 24bpp DXGI formats aka D3DFMT_R8G8B8
				printf("Error: No 24bpp DXGI formats aka D3DFMT_R8G8B8.\n");
				TI_ASSERT(0);
				break;

			case 16:
				if (ISBITMASK(0x7c00, 0x03e0, 0x001f, 0x8000))
				{
					return DXGI_FORMAT_B5G5R5A1_UNORM;
				}
				if (ISBITMASK(0xf800, 0x07e0, 0x001f, 0x0000))
				{
					return DXGI_FORMAT_B5G6R5_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x7c00,0x03e0,0x001f,0x0000) aka D3DFMT_X1R5G5B5

				if (ISBITMASK(0x0f00, 0x00f0, 0x000f, 0xf000))
				{
					return DXGI_FORMAT_B4G4R4A4_UNORM;
				}

				// No DXGI format maps to ISBITMASK(0x0f00,0x00f0,0x000f,0x0000) aka D3DFMT_X4R4G4B4

				// No 3:3:2, 3:3:2:8, or paletted DXGI formats aka D3DFMT_A8R3G3B2, D3DFMT_R3G3B2, D3DFMT_P8, D3DFMT_A8P8, etc.
				break;
			}
		}
		else if (ddpf.flags & DDS_LUMINANCE)
		{
			if (8 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}

				// No DXGI format maps to ISBITMASK(0x0f,0x00,0x00,0xf0) aka D3DFMT_A4L4
			}

			if (16 == ddpf.RGBBitCount)
			{
				if (ISBITMASK(0x0000ffff, 0x00000000, 0x00000000, 0x00000000))
				{
					return DXGI_FORMAT_R16_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
				if (ISBITMASK(0x000000ff, 0x00000000, 0x00000000, 0x0000ff00))
				{
					return DXGI_FORMAT_R8G8_UNORM; // D3DX10/11 writes this out as DX10 extension
				}
			}
		}
		else if (ddpf.flags & DDS_ALPHA)
		{
			if (8 == ddpf.RGBBitCount)
			{
				return DXGI_FORMAT_A8_UNORM;
			}
		}
		else if (ddpf.flags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', 'T', '1') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC1_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '3') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '5') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			// While pre-mulitplied alpha isn't directly supported by the DXGI formats,
			// they are basically the same as these BC formats so they can be mapped
			if (MAKEFOURCC('D', 'X', 'T', '2') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC2_UNORM;
			}
			if (MAKEFOURCC('D', 'X', 'T', '4') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC3_UNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '1') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'U') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '4', 'S') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC4_SNORM;
			}

			if (MAKEFOURCC('A', 'T', 'I', '2') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'U') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_UNORM;
			}
			if (MAKEFOURCC('B', 'C', '5', 'S') == ddpf.fourCC)
			{
				return DXGI_FORMAT_BC5_SNORM;
			}

			// BC6H and BC7 are written using the "DX10" extended header

			if (MAKEFOURCC('R', 'G', 'B', 'G') == ddpf.fourCC)
			{
				return DXGI_FORMAT_R8G8_B8G8_UNORM;
			}
			if (MAKEFOURCC('G', 'R', 'G', 'B') == ddpf.fourCC)
			{
				return DXGI_FORMAT_G8R8_G8B8_UNORM;
			}

			if (MAKEFOURCC('Y', 'U', 'Y', '2') == ddpf.fourCC)
			{
				return DXGI_FORMAT_YUY2;
			}

			// Check for D3DFORMAT enums being set here
			switch (ddpf.fourCC)
			{
			case 36: // D3DFMT_A16B16G16R16
				return DXGI_FORMAT_R16G16B16A16_UNORM;

			case 110: // D3DFMT_Q16W16V16U16
				return DXGI_FORMAT_R16G16B16A16_SNORM;

			case 111: // D3DFMT_R16F
				return DXGI_FORMAT_R16_FLOAT;

			case 112: // D3DFMT_G16R16F
				return DXGI_FORMAT_R16G16_FLOAT;

			case 113: // D3DFMT_A16B16G16R16F
				return DXGI_FORMAT_R16G16B16A16_FLOAT;

			case 114: // D3DFMT_R32F
				return DXGI_FORMAT_R32_FLOAT;

			case 115: // D3DFMT_G32R32F
				return DXGI_FORMAT_R32G32_FLOAT;

			case 116: // D3DFMT_A32B32G32R32F
				return DXGI_FORMAT_R32G32B32A32_FLOAT;
			}
		}

		return DXGI_FORMAT_UNKNOWN;
	}


	//--------------------------------------------------------------------------------------
//    static DXGI_FORMAT MakeSRGB(DXGI_FORMAT format)
//    {
//        switch (format)
//        {
//        case DXGI_FORMAT_R8G8B8A8_UNORM:
//            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//
//        case DXGI_FORMAT_BC1_UNORM:
//            return DXGI_FORMAT_BC1_UNORM_SRGB;
//
//        case DXGI_FORMAT_BC2_UNORM:
//            return DXGI_FORMAT_BC2_UNORM_SRGB;
//
//        case DXGI_FORMAT_BC3_UNORM:
//            return DXGI_FORMAT_BC3_UNORM_SRGB;
//
//        case DXGI_FORMAT_B8G8R8A8_UNORM:
//            return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
//
//        case DXGI_FORMAT_B8G8R8X8_UNORM:
//            return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
//
//        case DXGI_FORMAT_BC7_UNORM:
//            return DXGI_FORMAT_BC7_UNORM_SRGB;
//
//        default:
//            return format;
//        }
//    }

	E_TEXTURE_TYPE GetTixTypeFromDdsType(uint32 ResDim, bool isCube)
	{
		switch (ResDim)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			return ETT_TEXTURE_1D;
		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if (isCube)
			{
				return ETT_TEXTURE_CUBE;
			}
			else
			{
				return ETT_TEXTURE_2D;
			}
		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			return ETT_TEXTURE_3D;
		}
		return ETT_TEXTURE_UNKNOWN;
	}

	E_PIXEL_FORMAT GetTixFormatFromDXGIFormat(DXGI_FORMAT DFormat)
	{
		switch (DFormat)
		{
		case DXGI_FORMAT_BC1_UNORM:
			return EPF_DDS_DXT1;
		case DXGI_FORMAT_BC3_UNORM:
			return EPF_DDS_DXT5;
		case DXGI_FORMAT_BC5_UNORM:
			return EPF_DDS_BC5;
		case DXGI_FORMAT_R8_UNORM:
			return EPF_A8;
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
			return EPF_RGBA32F;
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return EPF_RGBA8;
        default:
            return EPF_UNKNOWN;
		}
	}

	//--------------------------------------------------------------------------------------
	// Return the BPP for a particular format
	//--------------------------------------------------------------------------------------
	uint32 BitsPerPixel(DXGI_FORMAT fmt)
	{
		switch (fmt)
		{
		case DXGI_FORMAT_R32G32B32A32_TYPELESS:
		case DXGI_FORMAT_R32G32B32A32_FLOAT:
		case DXGI_FORMAT_R32G32B32A32_UINT:
		case DXGI_FORMAT_R32G32B32A32_SINT:
			return 128;

		case DXGI_FORMAT_R32G32B32_TYPELESS:
		case DXGI_FORMAT_R32G32B32_FLOAT:
		case DXGI_FORMAT_R32G32B32_UINT:
		case DXGI_FORMAT_R32G32B32_SINT:
			return 96;

		case DXGI_FORMAT_R16G16B16A16_TYPELESS:
		case DXGI_FORMAT_R16G16B16A16_FLOAT:
		case DXGI_FORMAT_R16G16B16A16_UNORM:
		case DXGI_FORMAT_R16G16B16A16_UINT:
		case DXGI_FORMAT_R16G16B16A16_SNORM:
		case DXGI_FORMAT_R16G16B16A16_SINT:
		case DXGI_FORMAT_R32G32_TYPELESS:
		case DXGI_FORMAT_R32G32_FLOAT:
		case DXGI_FORMAT_R32G32_UINT:
		case DXGI_FORMAT_R32G32_SINT:
		case DXGI_FORMAT_R32G8X24_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
		case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
		case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		case DXGI_FORMAT_Y416:
		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			return 64;

		case DXGI_FORMAT_R10G10B10A2_TYPELESS:
		case DXGI_FORMAT_R10G10B10A2_UNORM:
		case DXGI_FORMAT_R10G10B10A2_UINT:
		case DXGI_FORMAT_R11G11B10_FLOAT:
		case DXGI_FORMAT_R8G8B8A8_TYPELESS:
		case DXGI_FORMAT_R8G8B8A8_UNORM:
		case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		case DXGI_FORMAT_R8G8B8A8_UINT:
		case DXGI_FORMAT_R8G8B8A8_SNORM:
		case DXGI_FORMAT_R8G8B8A8_SINT:
		case DXGI_FORMAT_R16G16_TYPELESS:
		case DXGI_FORMAT_R16G16_FLOAT:
		case DXGI_FORMAT_R16G16_UNORM:
		case DXGI_FORMAT_R16G16_UINT:
		case DXGI_FORMAT_R16G16_SNORM:
		case DXGI_FORMAT_R16G16_SINT:
		case DXGI_FORMAT_R32_TYPELESS:
		case DXGI_FORMAT_D32_FLOAT:
		case DXGI_FORMAT_R32_FLOAT:
		case DXGI_FORMAT_R32_UINT:
		case DXGI_FORMAT_R32_SINT:
		case DXGI_FORMAT_R24G8_TYPELESS:
		case DXGI_FORMAT_D24_UNORM_S8_UINT:
		case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
		case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
		case DXGI_FORMAT_B8G8R8A8_TYPELESS:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		case DXGI_FORMAT_B8G8R8X8_TYPELESS:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		case DXGI_FORMAT_AYUV:
		case DXGI_FORMAT_Y410:
		case DXGI_FORMAT_YUY2:
			return 32;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			return 24;

		case DXGI_FORMAT_R8G8_TYPELESS:
		case DXGI_FORMAT_R8G8_UNORM:
		case DXGI_FORMAT_R8G8_UINT:
		case DXGI_FORMAT_R8G8_SNORM:
		case DXGI_FORMAT_R8G8_SINT:
		case DXGI_FORMAT_R16_TYPELESS:
		case DXGI_FORMAT_R16_FLOAT:
		case DXGI_FORMAT_D16_UNORM:
		case DXGI_FORMAT_R16_UNORM:
		case DXGI_FORMAT_R16_UINT:
		case DXGI_FORMAT_R16_SNORM:
		case DXGI_FORMAT_R16_SINT:
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_A8P8:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return 16;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
		case DXGI_FORMAT_NV11:
			return 12;

		case DXGI_FORMAT_R8_TYPELESS:
		case DXGI_FORMAT_R8_UNORM:
		case DXGI_FORMAT_R8_UINT:
		case DXGI_FORMAT_R8_SNORM:
		case DXGI_FORMAT_R8_SINT:
		case DXGI_FORMAT_A8_UNORM:
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
			return 8;

		case DXGI_FORMAT_R1_UNORM:
			return 1;

		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			return 4;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			return 8;

		default:
			return 0;
		}
	}

	//--------------------------------------------------------------------------------------
	// Get surface information for a particular format
	//--------------------------------------------------------------------------------------
	static void GetSurfaceInfo(
		int32 width,
		int32 height,
		DXGI_FORMAT fmt,
		uint32* outNumBytes,
		uint32* outRowBytes,
		uint32* outNumRows)
	{
		uint32 numBytes = 0;
		uint32 rowBytes = 0;
		uint32 numRows = 0;

		bool bc = false;
		bool packed = false;
		bool planar = false;
		uint32 bpe = 0;
		switch (fmt)
		{
		case DXGI_FORMAT_BC1_TYPELESS:
		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
		case DXGI_FORMAT_BC4_TYPELESS:
		case DXGI_FORMAT_BC4_UNORM:
		case DXGI_FORMAT_BC4_SNORM:
			bc = true;
			bpe = 8;
			break;

		case DXGI_FORMAT_BC2_TYPELESS:
		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
		case DXGI_FORMAT_BC3_TYPELESS:
		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
		case DXGI_FORMAT_BC5_TYPELESS:
		case DXGI_FORMAT_BC5_UNORM:
		case DXGI_FORMAT_BC5_SNORM:
		case DXGI_FORMAT_BC6H_TYPELESS:
		case DXGI_FORMAT_BC6H_UF16:
		case DXGI_FORMAT_BC6H_SF16:
		case DXGI_FORMAT_BC7_TYPELESS:
		case DXGI_FORMAT_BC7_UNORM:
		case DXGI_FORMAT_BC7_UNORM_SRGB:
			bc = true;
			bpe = 16;
			break;

		case DXGI_FORMAT_R8G8_B8G8_UNORM:
		case DXGI_FORMAT_G8R8_G8B8_UNORM:
		case DXGI_FORMAT_YUY2:
			packed = true;
			bpe = 4;
			break;

		case DXGI_FORMAT_Y210:
		case DXGI_FORMAT_Y216:
			packed = true;
			bpe = 8;
			break;

		case DXGI_FORMAT_NV12:
		case DXGI_FORMAT_420_OPAQUE:
			planar = true;
			bpe = 2;
			break;

		case DXGI_FORMAT_P010:
		case DXGI_FORMAT_P016:
			planar = true;
			bpe = 4;
			break;
        default:
            break;

		}

		if (bc)
		{
			uint32 numBlocksWide = 0;
			if (width > 0)
			{
				numBlocksWide = std::max<uint32>(1, (width + 3) / 4);
			}
			uint32 numBlocksHigh = 0;
			if (height > 0)
			{
				numBlocksHigh = std::max<uint32>(1, (height + 3) / 4);
			}
			rowBytes = numBlocksWide * bpe;
			numRows = numBlocksHigh;
			numBytes = rowBytes * numBlocksHigh;
		}
		else if (packed)
		{
			rowBytes = ((width + 1) >> 1) * bpe;
			numRows = height;
			numBytes = rowBytes * height;
		}
		else if (fmt == DXGI_FORMAT_NV11)
		{
			rowBytes = ((width + 3) >> 2) * 4;
			numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
			numBytes = rowBytes * numRows;
		}
		else if (planar)
		{
			rowBytes = ((width + 1) >> 1) * bpe;
			numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
			numRows = height + ((height + 1) >> 1);
		}
		else
		{
			uint32 bpp = BitsPerPixel(fmt);
			rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
			numRows = height;
			numBytes = rowBytes * height;
		}

		if (outNumBytes)
		{
			*outNumBytes = numBytes;
		}
		if (outRowBytes)
		{
			*outRowBytes = rowBytes;
		}
		if (outNumRows)
		{
			*outNumRows = numRows;
		}
	}
	//--------------------------------------------------------------------------------------
	static TResTextureDefine* CreateTextureFromDDS(
		const DDS_HEADER* header,
		const uint8* Data,
		uint32 DataSize,
		int32 LodBias)
	{
		uint32 width = header->width;
		uint32 height = header->height;
		uint32 depth = header->depth;

		uint32_t resDim = D3D12_RESOURCE_DIMENSION_UNKNOWN;
		uint32 arraySize = 1;
		DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
		bool isCubeMap = false;

		uint32 mipCount = header->mipMapCount;
		if (0 == mipCount)
		{
			mipCount = 1;
		}
		TI_ASSERT(mipCount > (uint32)LodBias);

		if ((header->ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
		{
			auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));

			arraySize = d3d10ext->arraySize;
			if (arraySize == 0)
			{
				return nullptr;
			}

			switch (d3d10ext->dxgiFormat)
			{
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
			case DXGI_FORMAT_A8P8:
				printf("DDS Loader: Unsupported dxgi Format: %d.\n", d3d10ext->dxgiFormat);
				return nullptr;

			default:
				if (BitsPerPixel(d3d10ext->dxgiFormat) == 0)
				{
					printf("DDS Loader: Unsupported dxgi Format: %d.\n", d3d10ext->dxgiFormat);
					return nullptr;
				}
			}

			format = d3d10ext->dxgiFormat;

			switch (d3d10ext->resourceDimension)
			{
			case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
				// D3DX writes 1D textures with a fixed Height of 1
				if ((header->flags & DDS_HEIGHT) && height != 1)
				{
					printf("DDS Loader: Invalid 1D texture with height: %d.\n", height);
					return nullptr;
				}
				height = depth = 1;
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
				if (d3d10ext->miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE)
				{
					arraySize *= 6;
					isCubeMap = true;
				}
				depth = 1;
				break;

			case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
				if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
				{
					printf("DDS Loader: Invalid texture 3D.\n");
					return nullptr;
				}

				if (arraySize > 1)
				{
					printf("DDS Loader: Not supported texture 3D.\n");
					return nullptr;
				}
				break;

			default:
				printf("DDS Loader: Not supported texture Dimension.\n");
				return nullptr;
			}

			resDim = d3d10ext->resourceDimension;
		}
		else
		{
			format = GetDXGIFormat(header->ddspf);

			if (format == DXGI_FORMAT_UNKNOWN)
			{
				printf("DDS Loader: Unknown format.\n");
				return nullptr;
			}

			if (header->flags & DDS_HEADER_FLAGS_VOLUME)
			{
				resDim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
			}
			else
			{
				if (header->caps2 & DDS_CUBEMAP)
				{
					// We require all six faces to be defined
					if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
					{
						printf("DDS Loader: Invalid cube map faces.\n");
						return nullptr;
					}

					arraySize = 6;
					isCubeMap = true;
				}

				depth = 1;
				resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

				// Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
			}

			assert(BitsPerPixel(format) != 0);
		}

		// Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
		if (mipCount > D3D12_REQ_MIP_LEVELS)
		{
			printf("DDS Loader: Mip level out of range. Mips: %d.\n", mipCount);
			return nullptr;
		}

		switch (resDim)
		{
		case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
			if ((arraySize > D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
				(width > D3D12_REQ_TEXTURE1D_U_DIMENSION))
			{
				printf("DDS Loader: Invalid dimension of texture 1D. ArraySize: %d; Width: %d.\n", arraySize, width);
				return nullptr;
			}
			break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
			if (isCubeMap)
			{
				// This is the right bound because we set arraySize to (NumCubes*6) above
				if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
					(width > D3D12_REQ_TEXTURECUBE_DIMENSION) ||
					(height > D3D12_REQ_TEXTURECUBE_DIMENSION))
				{
					printf("DDS Loader: Invalid dimension of texture Cube. ArraySize: %d; Width: %d; Height %d.\n", arraySize, width, height);
					return nullptr;
				}
			}
			else if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
				(width > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
				(height > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION))
			{
				printf("DDS Loader: Invalid dimension of texture 2D. ArraySize: %d; Width: %d; Height %d.\n", arraySize, width, height);
				return nullptr;
			}
			break;

		case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
			if ((arraySize > 1) ||
				(width > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
				(height > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
				(depth > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
			{
				printf("DDS Loader: Invalid dimension of texture 3D. ArraySize: %d; Width: %d; Height %d; Depth: %d.\n", arraySize, width, height, depth);
				return nullptr;
			}
			break;

		default:
			printf("DDS Loader: unknown dimension .\n");
			return nullptr;
		}

		{
			// Create the texture
			TResTextureDefine* Texture = ti_new TResTextureDefine();
			Texture->Desc.Type = GetTixTypeFromDdsType(resDim, (header->caps2 & DDS_CUBEMAP) != 0);
			if (Texture->Desc.Type == ETT_TEXTURE_UNKNOWN)
			{
				printf("Error: unknown texture type.\n");
				TI_ASSERT(0);
			}
			Texture->Desc.Format = GetTixFormatFromDXGIFormat(format);
			if (Texture->Desc.Format == EPF_UNKNOWN)
			{
				printf("Error: unknown texture pixel format.\n");
				TI_ASSERT(0);
			}
			Texture->Desc.Width = width >> LodBias;
			Texture->Desc.Height = height >> LodBias;
			Texture->Desc.AddressMode = ETC_REPEAT;
			Texture->Desc.SRGB = 0;
			Texture->Desc.Mips = mipCount - LodBias;

			Texture->ImageSurfaces.resize(arraySize);

			const uint8* SrcData = Data;
			uint32 NumBytes = 0;
			uint32 RowBytes = 0;
			//uint32 index = 0;
			for (uint32 j = 0; j < arraySize; j++)
			{
				uint32 w = width;
				uint32 h = height;
				uint32 d = depth;
				TImage * Image = ti_new TImage(Texture->Desc.Format, Texture->Desc.Width, Texture->Desc.Height);
				Texture->ImageSurfaces[j] = Image;
				if (Texture->Desc.Mips > 1)
				{
					Image->AllocEmptyMipmaps();
				}

				for (uint32 i = 0; i < mipCount; i++)
				{
					GetSurfaceInfo(w,
						h,
						format,
						&NumBytes,
						&RowBytes,
						nullptr
					);

					if (i >= (uint32)LodBias)
					{
						// Set data
						TImage::TSurfaceData& Surface = Image->GetMipmap(i - LodBias);
						TI_ASSERT(NumBytes == Surface.Data.GetBufferSize());
						memcpy(Surface.Data.GetBuffer(), SrcData, NumBytes);
					}

					SrcData += NumBytes * d;

					w = w >> 1;
					h = h >> 1;
					d = d >> 1;
					if (w == 0)
					{
						w = 1;
					}
					if (h == 0)
					{
						h = 1;
					}
					if (d == 0)
					{
						d = 1;
					}
				}
			}

			// Convert RGBA32F Image to RGBA16F
			if (Texture->Desc.Format == EPF_RGBA32F)
			{
				TStream Buffer;
				for (auto& Image : Texture->ImageSurfaces)
				{
					TImage * F32Image = Image;
					TImage * F16Image = ti_new TImage(EPF_RGBA16F, F32Image->GetWidth(), F32Image->GetHeight());
					F16Image->AllocEmptyMipmaps();
					Image = F16Image;
					for (int32 Mip = 0 ; Mip < F32Image->GetMipmapCount() ; ++ Mip)
					{
						TImage::TSurfaceData& F32Data = F32Image->GetMipmap(Mip);
						TImage::TSurfaceData& F16Data = F16Image->GetMipmap(Mip);
						F16Data.Data.Reset();

						const int32 DataNum = F32Data.Data.GetLength() / sizeof(float);
						const float* Data = (const float*)F32Data.Data.GetBuffer();
						for (int i = 0; i < DataNum; ++i)
						{
							half hf = half(Data[i]);
							F16Data.Data.Put(&hf, sizeof(half));
						}
					}
					ti_delete F32Image;
				}
				Texture->Desc.Format = EPF_RGBA16F;
			}
			return Texture;
		}
	}

	TResTextureDefine* TResTextureHelper::LoadDdsFile(const TResTextureSourceInfo& SrcInfo)
	{
		TFile f;
		TString SrcPathName = TResSettings::GlobalSettings.SrcPath + SrcInfo.TextureSource;
		if (!f.Open(SrcPathName, EFA_READ))
		{
			return nullptr;
		}

		// Load file to memory
		const int32 FileSize = f.GetSize();
		uint8* FileBuffer = ti_new uint8[FileSize];
		f.Read(FileBuffer, FileSize, FileSize);
		f.Close();

		// Parse dds file
		uint32 dwMagicNumber = *(const uint32*)(FileBuffer);
		if (dwMagicNumber != DDS_MAGIC)
		{
			ti_delete[] FileBuffer;
			return nullptr;
		}

		auto header = reinterpret_cast<const DDS_HEADER*>(FileBuffer + sizeof(uint32));

		// Verify header to validate DDS file
		if (header->size != sizeof(DDS_HEADER) ||
			header->ddspf.size != sizeof(DDS_PIXELFORMAT))
		{
			ti_delete[] FileBuffer;
			return nullptr;
		}

		int32 offset = sizeof(DDS_HEADER) + sizeof(uint32);

		// Check for extensions
		if (header->ddspf.flags & DDS_FOURCC)
		{
			if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
				offset += sizeof(DDS_HEADER_DXT10);
		}

		// Must be long enough for all headers and magic value
		if (FileSize < offset)
		{
			ti_delete[] FileBuffer;
			return nullptr;
		}

		TString Name, Path;
		GetPathAndName(SrcPathName, Path, Name);

		TResTextureDefine* Texture = CreateTextureFromDDS(header, FileBuffer + offset, FileSize - offset, SrcInfo.LodBias);
		Texture->Name = Name;
		Texture->Path = Path;

		ti_delete[] FileBuffer;
		return Texture;
	}

	TResTextureDefine* LoadTgaToDds(const TResTextureSourceInfo& SrcInfo)
	{
		// Load Tga Image to find its format and width/height
		TString SrcName = TResSettings::GlobalSettings.SrcPath + SrcInfo.TextureSource;
		int32 ImageW, ImageH;
		E_PIXEL_FORMAT ImageFormat;
		{
			TFile ImageFile;
			if (!ImageFile.Open(SrcName, EFA_READ))
			{
				return nullptr;
			}
			TImage* TgaImage = TImage::LoadImageTGA(ImageFile);
			ImageW = TgaImage->GetWidth();
			ImageH = TgaImage->GetHeight();
			ImageFormat = TgaImage->GetFormat();
			ti_delete TgaImage;
		}

		int32 TargetW = ImageW >> SrcInfo.LodBias;
		int32 TargetH = ImageH >> SrcInfo.LodBias;
		TI_ASSERT(TargetW > 0 && TargetH > 0);
		TString TargetFormatStr;
		if (SrcInfo.TargetFormat == EPF_UNKNOWN)
		{
			switch (ImageFormat)
			{
			case EPF_RGB8:
				TargetFormatStr = "DXT1";
				break;
			case EPF_RGBA8:
				TargetFormatStr = "DXT5";
				break;
			default:
				printf("Error : un-supported ImageFormat : %d.\n", ImageFormat);
				TI_ASSERT(0);
				break;
			}
		}
		else
		{
			switch (SrcInfo.TargetFormat)
			{
			case EPF_A8:
				TargetFormatStr = "R8_UNORM";
				break;
			case EPF_RGBA8:
				TargetFormatStr = "R8G8B8A8_UNORM";
				break;
			default:
				printf("Error : un-supported TargetFormat : %d.\n", SrcInfo.TargetFormat);
				TI_ASSERT(0);
				break;
			}
		}

		// Find DDS converter and do convert for LDR image
		TString CommandLineStr, DebugCommandLineStr;
		{
			TString ExePath = GetExecutablePath();

			stringstream CmdSS;
			CmdSS << "/texconv -pow2 -y -nologo -silent";
			CmdSS << " -w " << TargetW << " -h " << TargetH;
			CmdSS << " -f " << TargetFormatStr;
			if (!SrcInfo.HasMips)
			{
				CmdSS << " -m 1";
			}
			CmdSS << " " << SrcName;
			CommandLineStr = ExePath + CmdSS.str();
			DebugCommandLineStr = CmdSS.str();
		}
		printf("  Cmd : %s\n", DebugCommandLineStr.c_str());
		// Convert to DXT
		int ret = system(CommandLineStr.c_str());
		if (ret != 0)
		{
			printf("Error: failed to convert DXT file.\n");
			return nullptr;
		}

		TResTextureSourceInfo DxtSrcInfo = SrcInfo;
		TStringReplace(DxtSrcInfo.TextureSource, ".tga", ".DDS");
		TResTextureDefine * DxtTexture = TResTextureHelper::LoadDdsFile(DxtSrcInfo);

		TString DstPathName = TResSettings::GlobalSettings.SrcPath + DxtSrcInfo.TextureSource;
		DeleteTempFile(DstPathName);

		return DxtTexture;
	}

	TResTextureDefine* TResTextureHelper::ConvertToDds(TResTextureDefine* SrcImage)
	{
		E_PIXEL_FORMAT SrcFormat = SrcImage->ImageSurfaces[0]->GetFormat();
		E_PIXEL_FORMAT DstFormat = EPF_UNKNOWN;

		if (SrcFormat == EPF_RGB8)
		{
			printf("Error: Tga will never be EGB8 format.\n");
			TI_ASSERT(0);
			DstFormat = EPF_DDS_DXT1;
		}
		else if (SrcFormat == EPF_RGBA8)
		{
			if (SrcImage->TGASourcePixelDepth == 24 && !TResSettings::GlobalSettings.ForceAlphaChannel)
			{
				DstFormat = EPF_DDS_DXT1;
			}
			else
			{
				DstFormat = EPF_DDS_DXT5;
			}
		}
		else
		{
			// Do not need compress
			return SrcImage;
		}
		TResTextureDefine* DstImage = ti_new TResTextureDefine;
		DstImage->Name = SrcImage->Name;
		DstImage->Path = SrcImage->Path;
		DstImage->LodBias = SrcImage->LodBias;
		DstImage->Desc = SrcImage->Desc;
		DstImage->Desc.Format = DstFormat;

		DstImage->ImageSurfaces.resize(SrcImage->ImageSurfaces.size());


		for (int32 i = 0 ; i < (int32)SrcImage->ImageSurfaces.size() ; ++ i)
		{
			TImage * TgaImage = SrcImage->ImageSurfaces[i];
			TImage * DxtImage = ti_new TImage(DstFormat, TgaImage->GetWidth(), TgaImage->GetHeight());
			DstImage->ImageSurfaces[i] = DxtImage;
			if (TgaImage->GetMipmapCount() > 1)
			{
				DxtImage->AllocEmptyMipmaps();
			}

			for (int32 Mip = 0 ; Mip < DxtImage->GetMipmapCount(); ++ Mip)
			{
				const TImage::TSurfaceData& SrcData = TgaImage->GetMipmap(Mip);
				TImage::TSurfaceData& DstData = DxtImage->GetMipmap(Mip);

				rgba_surface Surface;
				Surface.ptr = (uint8_t*)SrcData.Data.GetBuffer();
				Surface.width = SrcData.W;
				Surface.height = SrcData.H;
				Surface.stride = SrcData.RowPitch;

				uint8_t* Dst = (uint8_t*)DstData.Data.GetBuffer();
				// Call ISPC function to convert
				if (DstFormat == EPF_DDS_DXT1)
				{
					CompressBlocksBC1(&Surface, Dst);
				}
				else if (DstFormat == EPF_DDS_DXT5)
				{
					CompressBlocksBC3(&Surface, Dst);
				}
			}
		}
		return DstImage;
	}

}
