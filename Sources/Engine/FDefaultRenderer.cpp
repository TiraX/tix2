/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
		// Create Zero reset command buffer
		CounterResetUniformBuffer = FRHI::Get()->CreateUniformBuffer(sizeof(uint32) * 4, 1, UB_FLAG_INTERMEDIATE);
		uint8 * ZeroData = ti_new uint8[sizeof(uint32) * 4];
		memset(ZeroData, 0, sizeof(uint32) * 4);
		FRHI::Get()->UpdateHardwareResourceUB(CounterResetUniformBuffer, ZeroData);
		ti_delete[] ZeroData;
	}

	void FDefaultRenderer::InitRenderFrame(FScene* Scene)
	{
		// Prepare frame view uniform buffer
		Scene->InitRenderFrame();

		TI_TODO("Remove draw list , use new gpu driven pipeline.");
		// Prepare frame primitive uniform buffers
		//for (int32 List = 0; List < LIST_COUNT; ++List)
		//{
		//	const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList((E_DRAWLIST_TYPE)List);
		//	for (auto& Primitive : Primitives)
		//	{
		//		if (Primitive->IsPrimitiveBufferDirty())
		//		{
		//			Primitive->UpdatePrimitiveBuffer_RenderThread();
		//		}
		//	}
		//}
	}

	void FDefaultRenderer::EndRenderFrame(FScene* Scene)
	{
		// Clear the flags in this frame.
		Scene->ClearSceneFlags();
	}

	void FDefaultRenderer::Render(FRHI* RHI, FScene* Scene)
	{
		RHI->BeginRenderToFrameBuffer();
		DrawSceneTiles(RHI, Scene);
	}

	void FDefaultRenderer::DrawSceneTiles(FRHI* RHI, FScene* Scene)
	{
		const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
		for (auto& TileIter : SceneTileResources)
		{
			const vector2di& TilePos = TileIter.first;
			FSceneTileResourcePtr TileRes = TileIter.second;

			const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
			for (uint32 PIndex = 0; PIndex < (uint32)TilePrimitives.size(); ++PIndex)
			{
				FPrimitivePtr Primitive = TilePrimitives[PIndex];

				if (Primitive != nullptr)
				{
					FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
					RHI->SetGraphicsPipeline(Primitive->GetPipeline());
					RHI->SetMeshBuffer(Primitive->GetMeshBuffer(), InstanceBuffer);
					ApplyShaderParameter(RHI, Scene, Primitive);

					RHI->DrawPrimitiveIndexedInstanced(
						Primitive->GetMeshBuffer(),
						InstanceBuffer == nullptr ? 1 : Primitive->GetInstanceCount(),
						Primitive->GetInstanceOffset());
				}
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
			RHI->SetArgumentBuffer(Argument.BindingIndex, Primitive->GetArgumentBuffer());
			break;
		default:
			TI_ASSERT(0);
			break;
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
	}
}
