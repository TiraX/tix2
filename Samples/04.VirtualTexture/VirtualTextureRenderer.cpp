/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "VirtualTextureRenderer.h"
#include "FVTSystem.h"
#include "FVTTaskThread.h"

static const int32 QuadTreeElementSize = sizeof(float);
static const int32 QuadTreeElementCount = FVTSystem::TotalPagesInVT;

FTileDeterminationCS::FTileDeterminationCS(int32 W, int32 H)
	: FComputeTask("S_TileDeterminationCS")
	, InputSize(W, H)
	, UVBufferTriggerd(false)
{
}

FTileDeterminationCS::~FTileDeterminationCS()
{}

void FTileDeterminationCS::Run(FRHI * RHI)
{
#if defined (TI_PLATFORM_IOS)
    RHI->SetComputePipeline(ComputePipeline);
    RHI->SetComputeTexture(0, ScreenUV);
    RHI->SetComputeBuffer(0, QuadTreeBuffer);
#elif defined (TI_PLATFORM_WIN32)
	RHI->SetResourceStateUB(QuadTreeBuffer, RESOURCE_STATE_COPY_DEST);

	RHI->SetComputePipeline(ComputePipeline);
    RHI->SetComputeArgumentBuffer(0, ComputeArgument);
#endif

    const int32 SamplerPerPixel = 4;
	RHI->DispatchCompute(vector3di(32, 32, 1), vector3di(int32(InputSize.X / ThreadBlockSize / SamplerPerPixel), int32(InputSize.Y / ThreadBlockSize / SamplerPerPixel), 1));
}

void FTileDeterminationCS::PrepareBuffers(FTexturePtr UVInput)
{
	// prepare compute parameters
	// create quad tree buffer to store tile info
    uint8 * ZeroData = ti_new uint8[QuadTreeElementSize * QuadTreeElementCount];
    memset(ZeroData, 0, QuadTreeElementSize * QuadTreeElementCount);
	QuadTreeBuffer = FRHI::Get()->CreateUniformBuffer(QuadTreeElementSize, QuadTreeElementCount, UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_READBACK);
	FRHI::Get()->UpdateHardwareResourceUB(QuadTreeBuffer, ZeroData);
	// create a zero inited buffer to clear quad tree buffer
	QuadTreeBufferClear = FRHI::Get()->CreateUniformBuffer(QuadTreeElementSize, QuadTreeElementCount, 0);
	FRHI::Get()->UpdateHardwareResourceUB(QuadTreeBufferClear, ZeroData);
	ti_delete[] ZeroData;
    
    ScreenUV = UVInput;
	
	// Create compute argument buffer
#if !defined (TI_PLATFORM_IOS)
    ComputeArgument = FRHI::Get()->CreateArgumentBuffer(2);
    ComputeArgument->SetTexture(0, UVInput);
    ComputeArgument->SetBuffer(1, QuadTreeBuffer);
    FRHI::Get()->UpdateHardwareResourceAB(ComputeArgument, ComputeShader);
#endif
}

void FTileDeterminationCS::PrepareDataForCPU(FRHI * RHI)
{
	RHI->PrepareDataForCPU(QuadTreeBuffer);
	UVBufferTriggerd = true;
}

TStreamPtr FTileDeterminationCS::ReadUVBuffer()
{
	if (UVBufferTriggerd)
	{
		TStreamPtr Result = QuadTreeBuffer->ReadBufferData();
		UVBufferTriggerd = false;
		return Result;
	}
	return nullptr;
}

void FTileDeterminationCS::ClearQuadTree(FRHI * RHI)
{
	// Clear quad tree.
	RHI->CopyBufferRegion(QuadTreeBuffer, 0, QuadTreeBufferClear, QuadTreeElementCount * QuadTreeElementSize);
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
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}

	if (FVTSystem::IsEnabled())
	{
		ComputeTileDetermination = ti_new FTileDeterminationCS(RTWidth, RTHeight);
        ComputeTileDetermination->Finalize();
		ComputeTileDetermination->PrepareBuffers(RT_BasePass->GetColorBuffer(ERTC_COLOR1).Texture);
	}
}

void FVirtualTextureRenderer::Render(FRHI* RHI, FScene* Scene)
{
	if (FVTSystem::IsEnabled())
	{
		TStreamPtr UVBuffer = ComputeTileDetermination->ReadUVBuffer();
		if (UVBuffer != nullptr)
		{
			FVTSystem::Get()->GetAnalysisThread()->AddUVBuffer(UVBuffer);
		}

		// Prepare virtual texture system indirect texture
		FVTSystem::Get()->PrepareVTIndirectTexture();

		// Clear quad tree in graphics list
		ComputeTileDetermination->ClearQuadTree(RHI);
	}

	// Render Base Pass
	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	RenderDrawList(RHI, Scene, LIST_OPAQUE);
	RenderDrawList(RHI, Scene, LIST_MASK);

	// Do UV discard check, only check when camera moved or primitives changed
	if (FVTSystem::IsEnabled())
	{
		//if (false)
		{
			RHI->BeginComputeTask();
			ComputeTileDetermination->Run(RHI);

			ComputeTileDetermination->PrepareDataForCPU(RHI);
			RHI->EndComputeTask();
		}
	}

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
