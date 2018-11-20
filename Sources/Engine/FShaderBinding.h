/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
#ifdef TIX_DEBUG
#	define DEBUG_SHADER_BINDING_TYPE 1
#else
#	define DEBUG_SHADER_BINDING_TYPE 0
#endif // TIX_DEBUG


	enum E_BINDING_TYPE
	{
		BINDING_UNIFORMBUFFER,
		BINDING_TEXTURE,

		BINDING_UNIFORMBUFFER_TABLE,
		BINDING_TEXTURE_TABLE,

		BINDING_TYPE_NUM,
		BINDING_TYPE_INVALID = BINDING_TYPE_NUM,
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
		FShaderBinding(uint32 InNumBindings, uint32 NumStaticSamplers);
		virtual ~FShaderBinding();

		virtual void InitBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType, uint32 InBindingRegisterIndex, uint32 InBindingSize, uint32 InBindingStage) = 0;
		virtual void InitStaticSampler(uint32 InBindingIndex, const FSamplerDesc& Desc, uint32 InBindingStage) = 0;

		virtual void Finalize(FRHI * RHI) = 0;

		const int32 GetNumBinding() const
		{
			return NumBindings;
		}

#if DEBUG_SHADER_BINDING_TYPE
		void ValidateBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType);
#endif

	protected:
#if DEBUG_SHADER_BINDING_TYPE
		void InitBindingType(uint32 InBindingIndex, E_BINDING_TYPE InBindingType);
#endif

	protected:
		int32 NumBindings;
#if DEBUG_SHADER_BINDING_TYPE
		TVector<int32> BindingTypes;
#endif
	};
}