/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FDefaultRenderer.h"
#include "FUniformBufferView.h"

namespace tix
{
	FDefaultRenderer::FDefaultRenderer()
	{
	}

	FDefaultRenderer::~FDefaultRenderer()
	{
		TI_ASSERT(IsRenderThread());
	}

	void FDefaultRenderer::InitInRenderThread()
	{
	}

	void FDefaultRenderer::Render(FRHI* RHI, FScene* Scene)
	{
		Scene->PrepareViewUniforms();

		const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList();
		for (const auto& Primitive : Primitives)
		{
			for (int32 m = 0; m < (int32)Primitive->MeshBuffers.size(); ++m)
			{
				FMeshBufferPtr MB = Primitive->MeshBuffers[m];
				FPipelinePtr PL = Primitive->Pipelines[m];
				FArgumentBufferPtr AB = Primitive->Arguments[m];

				RHI->SetMeshBuffer(MB);
				RHI->SetPipeline(PL);
				RHI->SetArgumentBuffer(AB);

				RHI->DrawPrimitiveIndexedInstanced(MB, 1);
			}
		}
	}

	void FDefaultRenderer::BindEngineBuffer(FRHI * RHI, E_SHADER_STAGE ShaderStage, const FShaderBinding::FShaderArgument& Argument, FScene * Scene, FPrimitivePtr Primitive)
	{
		switch (Argument.ArgumentType)
		{
		case ARGUMENT_EB_VIEW:
			RHI->SetUniformBuffer(ShaderStage, Argument.BindingIndex, Scene->GetViewUniformBuffer()->UniformBuffer);
			break;
		case ARGUMENT_EB_PRIMITIVE:
			TI_ASSERT(Primitive != nullptr);
			RHI->SetUniformBuffer(ShaderStage, Argument.BindingIndex, Primitive->PrimitiveUniformBuffer->UniformBuffer);
			break;
		case ARGUMENT_EB_LIGHTS:
			RHI->SetUniformBuffer(ShaderStage, Argument.BindingIndex, Scene->GetSceneLights()->GetSceneLightsUniform()->UniformBuffer);
			break;
		case ARGUMENT_MI_BUFFER:
		case ARGUMENT_MI_TEXTURE:
			break;
		default:
			TI_ASSERT(0);
			break;
		}
		TI_TODO("Bind VS with different Argument from PS");
	}

	void FDefaultRenderer::BindMaterialInstanceArgument(FRHI * RHI, FArgumentBufferPtr ArgumentBuffer)
	{
		RHI->SetArgumentBuffer(ArgumentBuffer);
	}

	void FDefaultRenderer::ApplyShaderParameter(FRHI * RHI, FScene * Scene, FPrimitivePtr Primitive, int32 MeshSection)
	{
		FPipelinePtr PL = Primitive->Pipelines[MeshSection];
		FShaderBindingPtr ShaderBinding = PL->GetShader()->ShaderBinding;

		// bind vertex arguments
		const TVector<FShaderBinding::FShaderArgument>& VSArguments = ShaderBinding->GetVertexShaderArguments();
		for (const auto& Arg : VSArguments)
		{
			BindEngineBuffer(RHI, ESS_VERTEX_SHADER, Arg, Scene, Primitive);
		}

		// bind pixel arguments
		const TVector<FShaderBinding::FShaderArgument>& PSArguments = ShaderBinding->GetPixelShaderArguments();
		for (const auto& Arg : PSArguments)
		{
			BindEngineBuffer(RHI, ESS_PIXEL_SHADER, Arg, Scene, Primitive);
		}

		FArgumentBufferPtr AB = Primitive->Arguments[MeshSection];
		BindMaterialInstanceArgument(RHI, AB);
	}

	void FDefaultRenderer::ApplyShaderParameter(FRHI * RHI, FShaderPtr Shader, FScene * Scene, FArgumentBufferPtr ArgumentBuffer)
	{
		FShaderBindingPtr ShaderBinding = Shader->ShaderBinding;

		// bind vertex arguments
		const TVector<FShaderBinding::FShaderArgument>& VSArguments = ShaderBinding->GetVertexShaderArguments();
		for (const auto& Arg : VSArguments)
		{
			BindEngineBuffer(RHI, ESS_VERTEX_SHADER, Arg, Scene, nullptr);
		}

		// bind pixel arguments
		const TVector<FShaderBinding::FShaderArgument>& PSArguments = ShaderBinding->GetPixelShaderArguments();
		for (const auto& Arg : PSArguments)
		{
			BindEngineBuffer(RHI, ESS_PIXEL_SHADER, Arg, Scene, nullptr);
		}

		BindMaterialInstanceArgument(RHI, ArgumentBuffer);
	}
}
