/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	// Dx12 need uniform for data, resource table for textures
	class FArgumentBufferDx12 : public FArgumentBuffer
	{
	public:
		FArgumentBufferDx12(FShaderPtr InShader);
		virtual ~FArgumentBufferDx12();
	protected:

	private:
		FUniformBufferPtr UniformBuffer;
		FRenderResourceTablePtr TextureResourceTable;

		friend class FRHIDx12;
	};
}

#endif	// COMPILE_WITH_RHI_DX12
