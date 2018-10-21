/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_BINDING_TYPE
	{
		BINDING_UNIFORMBUFFER,
		BINDING_TEXTURE,

		BINDING_TYPE_NUM,
	};

	struct FSamplerDesc
	{
		E_TEXTURE_FILTER_TYPE Filter;
		E_TEXTURE_ADDRESS_MODE AddressMode;
	};

	class FRHI;
	class FShaderBinding : public IReferenceCounted
	{
	public:
		FShaderBinding(uint32 NumBindings, uint32 NumStaticSamplers);
		virtual ~FShaderBinding();

		virtual void InitBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, uint32 InBindingRegisterIndex, uint32 InBindingStage) = 0;
		virtual void InitTableBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, uint32 InBindingRegisterIndex, uint32 InBindingSize, uint32 InBindingStage) = 0;
		virtual void InitStaticSampler(uint32 InBindingIndex, const FSamplerDesc& Desc, uint32 InBindingStage) = 0;

		virtual void Finalize(FRHI * RHI) = 0;

		virtual void Bind(FRHI * RHI, uint32 BindingIndex, FUniformBufferPtr UniformBuffer) = 0;
		virtual void Bind(FRHI * RHI, uint32 BindingIndex, FTexturePtr Texture) = 0;
		virtual void Bind(FRHI * RHI, uint32 BindingIndex, FRenderResourceTablePtr RenderResourceTable) = 0;

	private:
	};
}