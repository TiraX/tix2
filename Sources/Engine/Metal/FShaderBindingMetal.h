/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	class FShaderBindingMetal : public FShaderBinding
	{
	public:
		FShaderBindingMetal(uint32 NumRootParams = 0, uint32 NumStaticSamplers = 0)
			: FShaderBinding(NumRootParams)
		{
		}

		virtual ~FShaderBindingMetal()
		{
			TI_ASSERT(IsRenderThread());
		}

		virtual void InitBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, uint32 InBindingRegisterIndex, uint32 InBindingSize, uint32 InBindingStage) override;
		virtual void InitStaticSampler(uint32 InBindingIndex, const FSamplerDesc& Desc, uint32 InBindingStage) override;

		virtual void Finalize(FRHI * RHI) override;
	private:
        
	private:
		friend class FRHIMetal;
	};
}
#endif	// COMPILE_WITH_RHI_METAL
