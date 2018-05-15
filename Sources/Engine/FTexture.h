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
		FTexture(E_RESOURCE_FAMILY InFamily, TImagePtr InSourceImage);
		FTexture(E_RESOURCE_FAMILY InFamily, E_PIXEL_FORMAT InFormat, int32 InWidth, int32 InHeight);
		virtual ~FTexture();

		int32 GetWidth() const
		{
			return Width;
		}
		int32 GetHeight() const
		{
			return Height;
		}
		TImagePtr GetSourceImage() const
		{
			return SourceImage;
		}

	protected:
		E_PIXEL_FORMAT	Format;
		int32			Width;
		int32			Height;
		TImagePtr		SourceImage;


	protected:
	};
	typedef TI_INTRUSIVE_PTR(FTexture) FTexturePtr;
}
