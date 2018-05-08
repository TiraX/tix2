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
		virtual ~FTexture();

	protected:

	protected:
	};
	typedef TI_INTRUSIVE_PTR(FTexture) FTexturePtr;
}
