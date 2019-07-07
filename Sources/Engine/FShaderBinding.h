/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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
		// Material Binding
		BINDING_UNIFORMBUFFER,

		BINDING_UNIFORMBUFFER_TABLE,
		BINDING_TEXTURE_TABLE,

		BINDING_TYPE_NUM,
		BINDING_TYPE_INVALID = BINDING_TYPE_NUM,
	};

	enum E_ARGUMENT_TYPE
	{
		ARGUMENT_EB_VIEW,
		ARGUMENT_EB_PRIMITIVE,
		ARGUMENT_EB_LIGHTS,
		ARGUMENT_EB_INDIRECTTEXTURE,
		ARGUMENT_MI_BUFFER,
		ARGUMENT_MI_TEXTURE,

		ARGUMENT_UNKNOWN,
	};

	struct FSamplerDesc
	{
		E_TEXTURE_FILTER_TYPE Filter;
		E_TEXTURE_ADDRESS_MODE AddressMode;
	};

	class FRHI;
	class FShaderBinding : public FRenderResource
	{
	public:
		FShaderBinding(uint32 InNumBindings);
		virtual ~FShaderBinding();

		static E_ARGUMENT_TYPE GetArgumentTypeByName(const TString& ArgName, bool bIsTexture = false);

		const int32 GetNumBinding() const
		{
			return NumBindings;
		}

#if DEBUG_SHADER_BINDING_TYPE
		void ValidateBinding(uint32 InBindingIndex, E_BINDING_TYPE InBindingType);
#endif
		struct FShaderArgument
		{
			FShaderArgument(int32 InBindingIndex, E_ARGUMENT_TYPE InArgumentType)
				: BindingIndex(InBindingIndex)
				, ArgumentType(InArgumentType)
			{}

			bool operator < (const FShaderArgument& Other) const
			{
				return BindingIndex < Other.BindingIndex;
			}

			int32 BindingIndex;
			E_ARGUMENT_TYPE ArgumentType;
		};

		void AddShaderArgument(E_SHADER_STAGE ShaderStage, const FShaderArgument& InArgument);
		void SortArguments();

		const TVector<FShaderArgument>& GetVertexShaderArguments() const
		{
			return VertexArguments;
		}

		const TVector<FShaderArgument>& GetPixelShaderArguments() const
		{
			return PixelArguments;
		}

		int32 GetFirstPSBindingIndexByType(E_ARGUMENT_TYPE InType) const;

	protected:
#if DEBUG_SHADER_BINDING_TYPE
		void InitBindingType(uint32 InBindingIndex, E_BINDING_TYPE InBindingType);
#endif

	protected:
		int32 NumBindings;
		TVector<FShaderArgument> VertexArguments;
		TVector<FShaderArgument> PixelArguments;
#if DEBUG_SHADER_BINDING_TYPE
		TVector<int32> BindingTypes;
#endif
	};
}
