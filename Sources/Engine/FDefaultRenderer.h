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
		virtual void Render(FRHI* RHI, FScene* Scene) override;

		virtual void ApplyShaderParameter(FRHI * RHI, FScene * Scene, FPrimitivePtr Primitive, int32 MeshSection);
		virtual void ApplyShaderParameter(FRHI * RHI, FShaderPtr Shader, FScene * Scene, FArgumentBufferPtr ArgumentBuffer);
	protected:
		void BindEngineBuffer(FRHI * RHI, E_SHADER_STAGE ShaderStage, const FShaderBinding::FShaderArgument& Argument, FScene * Scene, FPrimitivePtr Primitive);
		void BindMaterialInstanceArgument(FRHI * RHI, FArgumentBufferPtr ArgumentBuffer);

	protected:
	};
}
