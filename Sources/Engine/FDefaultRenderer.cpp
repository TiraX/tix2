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

	void FDefaultRenderer::InitRenderFrame(FScene* Scene)
	{
		// Prepare frame view uniform buffer
		Scene->PrepareViewUniforms();

		// Prepare frame primitive uniform buffers
		for (int32 List = 0; List < LIST_COUNT; ++List)
		{
			const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList((E_DRAWLIST_TYPE)List);
			for (auto& Primitive : Primitives)
			{
				if (Primitive->IsPrimitiveBufferDirty())
				{
					Primitive->UpdatePrimitiveBuffer_RenderThread();
				}
			}
		}
	}

	void FDefaultRenderer::EndRenderFrame(FScene* Scene)
	{
		// Clear the flags in this frame.
		Scene->ClearSceneFlags();
	}

	void FDefaultRenderer::Render(FRHI* RHI, FScene* Scene)
	{
		RenderDrawList(RHI, Scene, LIST_OPAQUE);
	}

	void FDefaultRenderer::RenderDrawList(FRHI* RHI, FScene* Scene, E_DRAWLIST_TYPE ListType)
	{
		const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList(ListType);
		for (const auto& Primitive : Primitives)
		{
			FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
			RHI->SetGraphicsPipeline(Primitive->GetPipeline());
			RHI->SetMeshBuffer(Primitive->GetMeshBuffer(), InstanceBuffer);
			ApplyShaderParameter(RHI, Scene, Primitive);

			RHI->DrawPrimitiveIndexedInstanced(Primitive->GetMeshBuffer(), InstanceBuffer == nullptr ? 1 : InstanceBuffer->GetInstancesCount());
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
			RHI->SetUniformBuffer(ShaderStage, Argument.BindingIndex, Primitive->GetPrimitiveUniform()->UniformBuffer);
			break;
		case ARGUMENT_EB_LIGHTS:
			RHI->SetUniformBuffer(ShaderStage, Argument.BindingIndex, Scene->GetSceneLights()->GetSceneLightsUniform()->UniformBuffer);
			break;
#if (COMPILE_WITH_RHI_METAL)
        case ARGUMENT_EB_VT_INDIRECT:
            RHI->SetShaderTexture(Argument.BindingIndex, FVTSystem::Get()->GetVTIndirectTexture());
            break;
        case ARGUMENT_EB_VT_PHYSIC:
            RHI->SetShaderTexture(Argument.BindingIndex, FVTSystem::Get()->GetVTPhysicTexture());
            break;
#else
		case ARGUMENT_EB_VT_INDIRECT_AND_PHYSIC:
			RHI->SetArgumentBuffer(Argument.BindingIndex, FVTSystem::Get()->GetVTResource());
			break;
#endif
		case ARGUMENT_MI_ARGUMENTS:
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	void FDefaultRenderer::BindMaterialInstanceArgument(FRHI * RHI, FShaderBindingPtr InShaderBinding, FArgumentBufferPtr ArgumentBuffer)
	{
        int32 BindingIndex = InShaderBinding->GetPixelArgumentBufferBindingIndex();
		if (BindingIndex >= 0)
		{
			RHI->SetArgumentBuffer(BindingIndex, ArgumentBuffer);
		}
	}

	void FDefaultRenderer::ApplyShaderParameter(FRHI * RHI, FScene * Scene, FPrimitivePtr Primitive)
	{
		FShaderBindingPtr ShaderBinding = Primitive->GetPipeline()->GetShader()->ShaderBinding;

		// bind vertex arguments
		const TVector<FShaderBinding::FShaderArgument>& VSArguments = ShaderBinding->GetVertexComputeShaderArguments();
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

		TI_TODO("Bind VS with different Argument from PS");
		BindMaterialInstanceArgument(RHI, ShaderBinding, Primitive->GetArgumentBuffer());
	}

	void FDefaultRenderer::ApplyShaderParameter(FRHI * RHI, FShaderPtr Shader, FScene * Scene, FArgumentBufferPtr ArgumentBuffer)
	{
		FShaderBindingPtr ShaderBinding = Shader->ShaderBinding;

		// bind vertex arguments
		const TVector<FShaderBinding::FShaderArgument>& VSArguments = ShaderBinding->GetVertexComputeShaderArguments();
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

		TI_TODO("Bind VS with different Argument from PS");
		BindMaterialInstanceArgument(RHI, ShaderBinding, ArgumentBuffer);
	}
}
