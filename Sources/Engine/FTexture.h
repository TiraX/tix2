/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FTexture : public FRenderResourceInHeap
	{
	public:
		FTexture();
		FTexture(const TTextureDesc& Desc);
		virtual ~FTexture();

		virtual void Destroy() override {};

		void InitTextureInfo(TTexturePtr Texture);

		const TTextureDesc& GetDesc() const
		{
			return TextureDesc;
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
