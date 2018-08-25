/*
	TiX Engine v2.0 Copyright (C) 2018
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
		ETFT_NEAREST = 0,

		//! Bilinear texel filter, no mipmaps.
		ETFT_LINEAR,

		//! Nearest texel filter in nearest mipmap level.
		ETFT_NEAREST_MIPMAP_NEAREST,

		//! Bilinear texel filter in nearest mipmap level.
		ETFT_LINEAR_MIPMAP_NEAREST,

		//! Interpolated nearest texel filter between mipmap levels.
		ETFT_NEAREST_MIPMAP_LINEAR,

		//! Trilinear texel filter.
		ETFT_LINEAR_MIPMAP_LINEAR,

		ETFT_COUNT,
		ETFT_UNKNOWN = ETFT_COUNT,
	};

	//! Texture coord clamp mode outside [0.0, 1.0]
	enum E_TEXTURE_CLAMP
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

	struct TTextureDesc
	{
		E_TEXTURE_TYPE Type;
		E_PIXEL_FORMAT Format;
		int32 Width;
		int32 Height;
		E_TEXTURE_CLAMP WrapMode;
		uint32 SRGB;
		uint32 Mips;

		TTextureDesc()
			: Type(ETT_TEXTURE_2D)
			, Format(EPF_UNKNOWN)
			, Width(0)
			, Height(0)
			, WrapMode(ETC_REPEAT)
			, SRGB(0)
			, Mips(1)
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
		TI_API void ClearSurfaceData();
	protected:

	protected:
		TTextureDesc Desc;
		TVector<TSurface*> Surfaces;
	};
}
