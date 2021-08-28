/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FShader : public FRenderResource
	{
	public:
		FShader(const TString& ShaderName, E_SHADER_TYPE InType);
		FShader(const TShaderNames& RenderShaderNames, E_SHADER_TYPE InType);
		virtual ~FShader();

		E_SHADER_TYPE GetShaderType() const
		{
			return Type;
		}

		const TString& GetShaderName(E_SHADER_STAGE Stage) const
		{
			return ShaderNames.ShaderNames[Stage];
		}

		const TString& GetComputeShaderName() const
		{
			return ShaderNames.ShaderNames[ESS_COMPUTE_SHADER];
		}

		const TString& GetRtxShaderLibName() const
		{
			return ShaderNames.ShaderNames[ESS_SHADER_LIB];
		}

		void SetShaderBinding(FShaderBindingPtr InShaderBinding)
		{
			ShaderBinding = InShaderBinding;
		}
		FShaderBindingPtr GetShaderBinding()
		{
			return ShaderBinding;
		}

		// Add Local Shader Binding, return index
		uint32 AddLocalShaderBinding(FShaderBindingPtr InShaderBinding)
		{
			LocalShaderBindings.push_back(InShaderBinding);
			return (uint32)(LocalShaderBindings.size() - 1);
		}
		FShaderBindingPtr GetLocalShaderBinding(uint32 Index)
		{
			TI_ASSERT(Index < (uint32)LocalShaderBindings.size());
			return LocalShaderBindings[Index];
		}
	protected:

	protected:
		TShaderNames ShaderNames;
		E_SHADER_TYPE Type;
		FShaderBindingPtr ShaderBinding;
		TVector<FShaderBindingPtr> LocalShaderBindings;
	};
}
