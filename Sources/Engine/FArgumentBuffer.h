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

#if ENABLE_VT_SYSTEM
		void SetTextureNames(const TVector<TString>& InTextureNames)
		{
			TextureNames = InTextureNames;
		}

		void SetTextureSizes(const TVector<vector2di>& InSizes)
		{
			TextureSizes = InSizes;
		}
#endif

	protected:

	protected:
		FShaderPtr Shader;

#if ENABLE_VT_SYSTEM
		// For VT System
		TVector<TString> TextureNames;
		TVector<vector2di> TextureSizes;
#endif
	};
}
