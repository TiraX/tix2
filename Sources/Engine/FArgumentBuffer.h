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
		FArgumentBuffer(FShaderPtr InShader);
		virtual ~FArgumentBuffer();

		FShaderPtr GetShader()
		{
			return Shader;
		}

		void SetTextureNames(const TVector<TString>& InTextureNames)
		{
			TextureNames = InTextureNames;
		}

		void SetTextureSizes(const TVector<vector2di>& InSizes)
		{
			TextureSizes = InSizes;
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
		FShaderPtr Shader;

		// For VT System
		TVector<TString> TextureNames;
		TVector<vector2di> TextureSizes;
	};
}
