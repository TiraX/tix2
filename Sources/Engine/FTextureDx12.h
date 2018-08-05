/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	class FTextureDx12 : public FTexture
	{
	public:
		FTextureDx12();
		FTextureDx12(E_PIXEL_FORMAT InFormat, int32 InWidth, int32 InHeight);
		virtual ~FTextureDx12();

		virtual void Destroy() override;
	protected:

	private:
		ComPtr<ID3D12Resource> TextureResource;
		uint32 TexDescriptor;
		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
