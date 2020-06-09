/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	//! Texture types.
	enum E_TEXTURE_TYPE
	{
		ETT_TEXTURE_1D,
		ETT_TEXTURE_2D,
		ETT_TEXTURE_3D,

		ETT_TEXTURE_CUBE,

		ETT_TEXTURE_UNKNOWN,
	};

	//! Texture filter types.
	enum E_TEXTURE_FILTER_TYPE
	{
		//! Nearest texel filter.
		ETFT_MINMAG_NEAREST_MIP_NEAREST = 0,

		//! Bilinear texel filter, no mipmaps.
		ETFT_MINMAG_LINEAR_MIP_NEAREST,
		
		//! Interpolated nearest texel filter between mipmap levels.
		ETFT_MINMAG_NEAREST_MIPMAP_LINEAR,

		//! Trilinear texel filter.
		ETFT_MINMAG_LINEAR_MIPMAP_LINEAR,

		ETFT_COUNT,
		ETFT_UNKNOWN = ETFT_COUNT,
	};

	//! Texture coord clamp mode outside [0.0, 1.0]
	enum E_TEXTURE_ADDRESS_MODE
	{
		//! Texture repeats
		ETC_REPEAT = 0,

		//! Texture is clamped to the edge pixel
		ETC_CLAMP_TO_EDGE,

		//! Texture is alternatingly mirrored (0..1..0..1..0..)
		ETC_MIRROR,

		ETC_COUNT,
		ETC_UNKNOWN = ETC_COUNT,
	};

	enum E_TEXTURE_PARAMETER
	{
		ETP_MIN_FILTER,
		ETP_MAG_FILTER,
		ETP_WRAP_S,
		ETP_WRAP_T,
		ETP_WRAP_R,

		ETP_COUNT,
	};

	enum E_TEXTURE_FLAG
	{
		ETF_NONE = 0,
		ETF_RT_COLORBUFFER = 1 << 0,
		ETF_RT_DSBUFFER = 1 << 1,
		ETF_UAV = 1 << 2,

		ETF_RENDER_RESOURCE_UPDATED = 1 << 3,	// Used for FTexture
        
        // Used for iOS Metal
        ETF_MEMORY_LESS = 1 << 4,
	};

	struct TTextureDesc
	{
		E_TEXTURE_TYPE Type;
		E_PIXEL_FORMAT Format;
		int32 Width;
		int32 Height;
		E_TEXTURE_ADDRESS_MODE AddressMode;
		uint32 SRGB;
		uint32 Mips;
		uint32 Flags;

		TTextureDesc()
			: Type(ETT_TEXTURE_2D)
			, Format(EPF_UNKNOWN)
			, Width(0)
			, Height(0)
			, AddressMode(ETC_REPEAT)
			, SRGB(0)
			, Mips(1)
			, Flags(0)
		{}
	};

	class TTexture : public TResource
	{
	public:
		TTexture(const TTextureDesc& InDesc);
		virtual ~TTexture();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FTexturePtr TextureResource;

		class TSurface
		{
		public:
			TSurface()
				: Data(nullptr)
				, Width(0)
				, Height(0)
				, DataSize(0)
				, RowPitch(0)
			{}
			~TSurface()
			{
				SAFE_DELETE_ARRAY(Data);
				DataSize = 0;
			}

			uint8 * Data;
			uint32 Width;
			uint32 Height;
			uint32 DataSize;
			uint32 RowPitch;
		};
		TI_API void AddSurface(int32 Width, int32 Height, const uint8* Data, int32 RowPitch, int32 DataSize);
		TI_API const TTextureDesc& GetDesc() const
		{
			return Desc;
		}
		TI_API const TVector<TSurface*>& GetSurfaces() const
		{
			return  Surfaces;
		}
		TI_API void SetTextureFlag(E_TEXTURE_FLAG Flag, bool bEnable)
		{
			if (bEnable)
			{
				Desc.Flags |= Flag;
			}
			else
			{
				Desc.Flags &= ~Flag;
			}
		}
		TI_API void ClearSurfaceData();
	protected:

	protected:
		TTextureDesc Desc;
		TVector<TSurface*> Surfaces;
	};
}
