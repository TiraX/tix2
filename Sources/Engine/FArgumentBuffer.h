/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FArgumentBuffer : public FRenderResource
	{
	public:
		FArgumentBuffer(int32 ReservedTextures);
		virtual ~FArgumentBuffer();

		TI_API void SetDataBuffer(const void * InData, int32 DataLength);
		TI_API void SetTexture(int32 Index, FTexturePtr InTexture);

		void SetTextureNames(const TVector<TString>& InTextureNames)
		{
			TextureNames = InTextureNames;
		}

		void SetTextureSizes(const TVector<vector2di>& InSizes)
		{
			TextureSizes = InSizes;
		}

		const TStream& GetArgumentData() const
		{
			return ArgumentDataBuffer;
		}

		const TVector<FTexturePtr>& GetArgumentTextures() const
		{
			return ArgumentTextures;
		}

		const TVector<TString>& GetTextureNames() const
		{
			return TextureNames;
		}

		const TVector<vector2di>& GetTextureSizes() const
		{
			return TextureSizes;
		}

	protected:

	protected:
		TStream ArgumentDataBuffer;
		TVector<FTexturePtr> ArgumentTextures;

		// For VT System
		TVector<TString> TextureNames;
		TVector<vector2di> TextureSizes;
	};
}
