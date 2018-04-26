/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RHI_TYPE
	{
		ERHI_DX12 = 0,

		ERHI_NUM,
	};

	// Render hardware interface
	class FRHI
	{
	public: 
		static FRHI* CreateRHI(E_RHI_TYPE RhiType);
		virtual ~FRHI();

		virtual void ClearBuffers() = 0;
	protected:
		FRHI();

	private:
	};
}
