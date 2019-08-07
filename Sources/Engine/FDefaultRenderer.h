/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRHI;
	class FScene;

	// Default Renderer
	class TI_API FDefaultRenderer : public FRenderer
	{
	public:
		FDefaultRenderer();
		virtual ~FDefaultRenderer();

		virtual void InitInRenderThread() override;
		virtual void InitRenderFrame(FScene* Scene) override;
		virtual void EndRenderFrame(FScene* Scene) override;
		virtual void Render(FRHI* RHI, FScene* Scene) override;

		virtual void ApplyShaderParameter(FRHI * RHI, FScene * Scene, FPrimitivePtr Primitive);
		virtual void ApplyShaderParameter(FRHI * RHI, FShaderPtr Shader, FScene * Scene, FArgumentBufferPtr ArgumentBuffer);
	protected:
		void RenderDrawList(FRHI* RHI, FScene* Scene, E_DRAWLIST_TYPE ListType);
		void BindEngineBuffer(FRHI * RHI, E_SHADER_STAGE ShaderStage, const FShaderBinding::FShaderArgument& Argument, FScene * Scene, FPrimitivePtr Primitive);
		void BindMaterialInstanceArgument(FRHI * RHI, FShaderBindingPtr InShaderBinding, FArgumentBufferPtr ArgumentBuffer);

	protected:
	};
}
