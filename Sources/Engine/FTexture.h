/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FTexture : public FRenderResource
	{
	public:
		FTexture(E_RESOURCE_FAMILY InFamily);
		FTexture(E_RESOURCE_FAMILY InFamily, E_PIXEL_FORMAT InFormat, int32 InWidth, int32 InHeight);
		virtual ~FTexture();

		virtual void Destroy() override {};

		void InitTextureInfo(TTexturePtr Image);

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
