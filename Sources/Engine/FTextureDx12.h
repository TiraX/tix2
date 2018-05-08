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
		virtual ~FTextureDx12();

	protected:

	private:

		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
