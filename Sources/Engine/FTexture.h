/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FTexture : public FRenderResource
	{
	public:
		FTexture();
		FTexture(const TTextureDesc& Desc);
		virtual ~FTexture();

		void InitTextureInfo(TTexturePtr Texture);

		const TTextureDesc& GetDesc() const
		{
			return TextureDesc;
		}

		bool HasTextureFlag(E_TEXTURE_FLAG Flag)
		{
			return (TextureDesc.Flags & Flag) != 0;
		}
		
		void SetTextureFlag(E_TEXTURE_FLAG Flag, bool bEnable)
		{
			if (bEnable)
			{
				TextureDesc.Flags |= Flag;
			}
			else
			{
				TextureDesc.Flags &= ~Flag;
			}
		}

		int32 GetWidth() const
		{
			return TextureDesc.Width;
		}
		int32 GetHeight() const
		{
			return TextureDesc.Height;
		}

	protected:
		TTextureDesc TextureDesc;


	protected:
	};
}
