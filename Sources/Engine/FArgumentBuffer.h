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
		static const int32 MaxResourcesInArgumentBuffer = 16;
		FArgumentBuffer(int32 ReservedSlots);
		virtual ~FArgumentBuffer();

		TI_API void SetBuffer(int32 Index, FUniformBufferPtr InUniform);
		TI_API void SetTexture(int32 Index, FTexturePtr InTexture);

		void SetTextureNames(const TVector<TString>& InTextureNames)
		{
			TextureNames = InTextureNames;
		}

		void SetTextureSizes(const TVector<vector2di>& InSizes)
		{
			TextureSizes = InSizes;
		}

		const TVector<FRenderResourcePtr>& GetArguments() const
		{
			return Arguments;
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
		TVector<FRenderResourcePtr> Arguments;

		// For VT System
		TVector<TString> TextureNames;
		TVector<vector2di> TextureSizes;
	};
}
