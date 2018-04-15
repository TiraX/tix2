/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
namespace tix
{
	// Render hardware interface use DirectX 12
	class FRHIDx12 : public FRHI
	{
	public:
		virtual ~FRHIDx12();

	protected: 
		FRHIDx12();

	private:

		friend class FRHI;
	};
}
#endif	// COMPILE_WITH_RHI_DX12