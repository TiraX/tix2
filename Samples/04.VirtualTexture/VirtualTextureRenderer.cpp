/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "VirtualTextureRenderer.h"
#include "FVTSystem.h"
#include "FVTTaskThread.h"

FComputeUVDiscard::FComputeUVDiscard(int32 W, int32 H)
	: FComputeTask("S_UVDiscardCS")
	, InputSize(W, H)
	, UVBufferTriggerd(false)
{
}

FComputeUVDiscard::~FComputeUVDiscard()
{}

void FComputeUVDiscard::FinalizeInRenderThread()
{
	FComputeTask::FinalizeInRenderThread();

	// Init Constant Buffer
	InputInfoBuffer = ti_new FUVDiscardInput;
	InputInfoBuffer->UniformBufferData[0].Info.X = float(InputSize.X / ThreadBlockSize);	// Group rows
	InputInfoBuffer->InitUniformBuffer();
}

void FComputeUVDiscard::Run(FRHI * RHI)
{
	//uint32 CounterOffset = ProcessedCommandsBuffer->GetStructureSizeInBytes() * ProcessedCommandsBuffer->GetElements();
	//RHI->ComputeCopyBuffer(ProcessedCommandsBuffer, CounterOffset, ResetBuffer->UniformBuffer, 0, sizeof(uint32));

	RHI->SetResourceStateUB(OutputUVBuffer, RESOURCE_STATE_COPY_DEST);

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, InputInfoBuffer->UniformBuffer);
	RHI->SetComputeResourceTable(1, ResourceTable);

	RHI->DispatchCompute(uint32(InputSize.X / ThreadBlockSize), uint32(InputSize.Y / ThreadBlockSize), 1);
}

void FComputeUVDiscard::PrepareBuffers(FTexturePtr UVInput)
{
	// prepare compute parameters
	const int32 BufferSize = InputSize.X / ThreadBlockSize * InputSize.Y / ThreadBlockSize;
	TI_TODO("Try if OutputUVBuffer can use uint4 instead of ffloat4.");
	OutputUVBuffer = FRHI::Get()->CreateUniformBuffer(16, BufferSize, UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_READBACK);
	FRHI::Get()->UpdateHardwareResourceUB(OutputUVBuffer, nullptr);
	
	// Create Render Resource Table
	ResourceTable = FRHI::Get()->CreateRenderResourceTable(2, EHT_SHADER_RESOURCE);
	ResourceTable->PutTextureInTable(UVInput, 0);
	ResourceTable->PutBufferInTable(OutputUVBuffer, 1);
}

void FComputeUVDiscard::PrepareDataForCPU(FRHI * RHI)
{
	RHI->PrepareDataForCPU(OutputUVBuffer);
	UVBufferTriggerd = true;
}

TStreamPtr FComputeUVDiscard::ReadUVBuffer()
{
	if (UVBufferTriggerd)
	{
		TStreamPtr Result = OutputUVBuffer->ReadBufferData();
		UVBufferTriggerd = false;
		return Result;
	}
	return nullptr;
}

////////////////////////////////////////////////////////

FVirtualTextureRenderer::FVirtualTextureRenderer()
{
}

FVirtualTextureRenderer::~FVirtualTextureRenderer()
{
}

void FVirtualTextureRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
	RHI->InitCommandLists(GraphicsCount, ComputeCount);
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

	if (FVTSystem::IsEnabled())
	{
		// Second for render uv onto it.
		RT_BasePass->AddColorBuffer(EPF_RGBA16F, ERTC_COLOR1, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	}
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->Compile();

	AB_Result = RHI->CreateArgumentBuffer(FSRender.GetFullScreenShader());
	{
		ArgumentValues->Reset();
		ArgumentTextures.clear();
		ArgumentTextures.push_back(RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, ArgumentValues, ArgumentTextures);
	}

	if (FVTSystem::IsEnabled())
	{
		ComputeUVDiscard = ti_new FComputeUVDiscard(RTWidth, RTHeight);
		ComputeUVDiscard->PrepareBuffers(RT_BasePass->GetColorBuffer(ERTC_COLOR1).Texture);
		ComputeUVDiscard->Finalize();
	}
}

void FVirtualTextureRenderer::Render(FRHI* RHI, FScene* Scene)
{
	if (FVTSystem::IsEnabled())
	{
		TStreamPtr UVBuffer = ComputeUVDiscard->ReadUVBuffer();
		if (UVBuffer != nullptr)
		{
			FVTSystem::Get()->GetAnalysisThread()->AddUVBuffer(UVBuffer);
		}


		// Prepare virtual texture system indirect texture
		FVTSystem::Get()->PrepareVTIndirectTexture();
	}

	// Render Base Pass
	RHI->BeginPopulateCommandList(EPL_GRAPHICS);
	RHI->PushRenderTarget(RT_BasePass, "BasePass");
	RenderDrawList(RHI, Scene, LIST_OPAQUE);
	RenderDrawList(RHI, Scene, LIST_MASK);
	RHI->PopRenderTarget();
	RHI->EndPopulateCommandList();

	// Do UV discard check, only check when camera moved or primitives changed
	if (FVTSystem::IsEnabled())
	{
		if (Scene->HasSceneFlag(FScene::ViewProjectionDirty) || Scene->HasSceneFlag(FScene::ScenePrimitivesDirty))
		{
			RHI->BeginPopulateCommandList(EPL_COMPUTE);
			ComputeUVDiscard->Run(RHI);

			ComputeUVDiscard->PrepareDataForCPU(RHI);
			RHI->EndPopulateCommandList();
		}
	}

	RHI->BeginPopulateCommandList(EPL_GRAPHICS);
	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
	RHI->EndPopulateCommandList();
}
