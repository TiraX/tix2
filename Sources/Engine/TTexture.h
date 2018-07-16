/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
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
		E_PIXEL_FORMAT Format;
		int32 Width;
		int32 Height;
		E_TEXTURE_CLAMP WrapModeS;
		E_TEXTURE_CLAMP WrapModeT;
		E_TEXTURE_CLAMP WrapModeR;
		uint32 SRGB;
		uint32 Mips;
		uint32 DataSize;
	};

	class FTexture;
	typedef TI_INTRUSIVE_PTR(FTexture) FTexturePtr;

	class TTexture : public IReferenceCounted
	{
	public:
		TTextureDesc Desc;
		FTexturePtr TextureResource;

		TI_API void AddSurface(int32 Width, int32 Height, const uint8* Data, uint32 DataSize, uint32 RowPitch);
	protected:
		class TSurface
		{
		public:
			TSurface()
				: Data(nullptr)
				, DataSize(0)
				, Pitch(0)
				, Width(0)
				, Height(0)
			{}
			~TSurface()
			{
				SAFE_DELETE_ARRAY(Data);
				DataSize = 0;
				Pitch = 0;
			}

			uint8 * Data;
			uint32 DataSize;
			uint32 Pitch;
			uint32 Width;
			uint32 Height;
		};

	protected:
		TVector<TSurface*> Surfaces;
	};

	typedef TI_INTRUSIVE_PTR(TTexture) TTexturePtr;

}
