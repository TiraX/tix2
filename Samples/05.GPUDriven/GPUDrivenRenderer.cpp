/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUDrivenRenderer.h"

FGPUDrivenRenderer::FGPUDrivenRenderer()
{
}

FGPUDrivenRenderer::~FGPUDrivenRenderer()
{
}

void FGPUDrivenRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 RTWidth = 1600;
	const int32 RTHeight = TEngine::AppInfo.Height * RTWidth / TEngine::AppInfo.Width;

	TStreamPtr ArgumentValues = ti_new TStream;
	TVector<FTexturePtr> ArgumentTextures;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(RTWidth, RTHeight);
#if defined (TIX_DEBUG)
	RT_BasePass->SetResourceName("BasePass");
#endif
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}

	// Load default pipeline
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetPtr DebugMaterialAsset = TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TResourcePtr DebugMaterialResource = DebugMaterialAsset->GetResourcePtr();
	TMaterialPtr DebugMaterial = static_cast<TMaterial*>(DebugMaterialResource.get());
	DebugPipeline = DebugMaterial->PipelineResource;

	// Init GPU command buffer
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.reserve(2);
	CommandStructure.push_back(GPU_COMMAND_SET_MESH_BUFFER);
	CommandStructure.push_back(GPU_COMMAND_DRAW_INDEXED);
	GPUCommandSignature = RHI->CreateGPUCommandSignature(DebugPipeline, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(GPUCommandSignature);
}

void FGPUDrivenRenderer::UpdateGPUCommandBuffer(FRHI* RHI, FScene * Scene)
{
	// Encode all draw command to GPU command buffer
	TI_TODO("Process opaque list for now, encode other list in future.");

	const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList(LIST_OPAQUE);
	const uint32 PrimsCount = (uint32)Primitives.size();
	if (PrimsCount == 0)
	{
		return;
	}
	GPUCommandBuffer = RHI->CreateGPUCommandBuffer(GPUCommandSignature, PrimsCount);
	for (uint32 i = 0 ; i < PrimsCount ; ++ i)
	{
		FPrimitivePtr Primitive = Primitives[i];
		FMeshBufferPtr MeshBuffer = Primitive->GetMeshBuffer();
		FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
		TI_ASSERT(MeshBuffer != nullptr && InstanceBuffer != nullptr);
		GPUCommandBuffer->EncodeSetMeshBuffer(i, GPU_COMMAND_SET_MESH_BUFFER, MeshBuffer, InstanceBuffer);
		GPUCommandBuffer->EncodeSetDrawIndexed(i,
			GPU_COMMAND_DRAW_INDEXED,
			MeshBuffer->GetIndicesCount(),
			InstanceBuffer->GetInstancesCount(),
			0, 0, 0);
		//RHI->SetGraphicsPipeline(Primitive->GetPipeline());
		//RHI->SetMeshBuffer(Primitive->GetMeshBuffer(), InstanceBuffer);
		//ApplyShaderParameter(RHI, Scene, Primitive);

		//RHI->DrawPrimitiveIndexedInstanced(Primitive->GetMeshBuffer(), InstanceBuffer == nullptr ? 1 : InstanceBuffer->GetInstancesCount());
	}
	RHI->UpdateHardwareResourceGPUCommandBuffer(GPUCommandBuffer);
}

void FGPUDrivenRenderer::DrawGPUCommandBuffer(FRHI * RHI)
{
	if (GPUCommandBuffer != nullptr)
	{
		RHI->ExecuteGPUCommands(GPUCommandBuffer);
	}
}

void FGPUDrivenRenderer::Render(FRHI* RHI, FScene* Scene)
{
	if (Scene->HasSceneFlag(FScene::ScenePrimitivesDirty))
	{
		// Update GPU Command Buffer
		UpdateGPUCommandBuffer(RHI, Scene);
	}
	// Render Base Pass
    RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	//RenderDrawList(RHI, Scene, LIST_OPAQUE);
	//RenderDrawList(RHI, Scene, LIST_MASK);
	DrawGPUCommandBuffer(RHI);

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
