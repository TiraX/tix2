/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "RTAORenderer.h"

static const int32 USE_HBAO = 0;
static const int32 USE_GTAO = 1;

static const int32 AO_METHOD = USE_GTAO;

FRTAORenderer::FRTAORenderer()
{
}

FRTAORenderer::~FRTAORenderer()
{
}

void FRTAORenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
}

void FRTAORenderer::InitInRenderThread()
{
	FDefaultRenderer::InitInRenderThread();

	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	const int32 RTWidth = 1600;
	const int32 RTHeight = TEngine::AppInfo.Height * RTWidth / TEngine::AppInfo.Width;

	TStreamPtr ArgumentValues = ti_new TStream;
	TVector<FTexturePtr> ArgumentTextures;

	// Setup base pass render target
	RT_BasePass = FRenderTarget::Create(RTWidth, RTHeight);
	RT_BasePass->SetResourceName("RT_BasePass");
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, 1, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, 1, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	// Load default pipeline
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetPtr DebugMaterialAsset = TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TResourcePtr DebugMaterialResource = DebugMaterialAsset->GetResourcePtr();
	TMaterialPtr DebugMaterial = static_cast<TMaterial*>(DebugMaterialResource.get());
	FPipelinePtr DebugPipeline = DebugMaterial->PipelineResource;

	//HBAOCompute = ti_new FHBAOCS();
	//HBAOCompute->Finalize();
	//HBAOCompute->PrepareResources(RHI);

	//GTAOCompute = ti_new FGTAOCS();
	//GTAOCompute->Finalize();
	//GTAOCompute->PrepareResources(RHI);

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}

	// Create GBuffer
	TTextureDesc TextureDesc;
	TextureDesc.Format = EPF_RGBA8;
	TextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;

#define CreateTextureResource(T) T=RHI->CreateTexture(TextureDesc); \
	T->SetTextureFlag(ETF_UAV, true); \
	T->SetResourceName(#T); \
	RHI->UpdateHardwareResourceTexture(T)

	TextureDesc.Width = RTWidth;
	TextureDesc.Height = RTWidth;
	CreateTextureResource(T_GBuffer[GBUFFER_COLOR]);
#undef CreateTextureResource

	// For path tracer
	// Create root signatures

	// Create RTX pipeline state object
	const TString ShaderPathTracer = "S_Pathtracer";
	TShaderPtr Shader = ti_new TShader(ShaderPathTracer);
	Shader->LoadShaderCode();
	Shader->AddEntry("MyRayGenShader");
	Shader->AddEntry("RayMiss");
	Shader->AddEntry("RayClosestHit");
	Shader->SetHitGroupShader(HITGROUP_CLOSEST_HIT, "RayClosestHit");
	Shader->ShaderResource = RHI->CreateRtxShaderLib(ShaderPathTracer);
	RHI->UpdateHardwareResourceShader(Shader->ShaderResource, Shader);

	TRtxPipelinePtr RtxPSODesc = ti_new TRtxPipeline();
	RtxPSODesc->SetShaderLib(Shader);
	RtxPSO = RHI->CreateRtxPipeline(Shader->ShaderResource);
	RHI->UpdateHardwareResourceRtxPL(RtxPSO, RtxPSODesc);

	// Create constant buffers

	// Build shader tables
}

void FRTAORenderer::DrawSceneTiles(FRHI* RHI, FScene * Scene)
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

void FRTAORenderer::Render(FRHI* RHI, FScene* Scene)
{
	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	DrawSceneTiles(RHI, Scene);


	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
