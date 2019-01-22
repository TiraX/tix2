/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TBindingParamInfo
	{
		int8 BindingType;
		int8 BindingStage;
		uint8 BindingRegister;
		uint8 BindingSize;

		TBindingParamInfo()
			: BindingType(0)
			, BindingStage(0)
			, BindingRegister(0)
			, BindingSize(0)
		{}
	};

	class TShaderBinding : public TResource
	{
	public:
		TShaderBinding();
		virtual ~TShaderBinding();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FShaderBindingPtr ShaderBindingResource;
	protected:

	protected:
		TVector<TBindingParamInfo> Bindings;

		friend class TResFile;
	};
}
