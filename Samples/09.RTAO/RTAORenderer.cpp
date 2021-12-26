/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "RTAORenderer.h"

static const int32 USE_HBAO = 0;
static const int32 USE_GTAO = 1;

static const int32 AO_METHOD = USE_GTAO;

FRTAORenderer* RTAORenderer = nullptr;
FRTAORenderer* FRTAORenderer::Get()
{
	return RTAORenderer;
}


FRTAORenderer::FRTAORenderer()
{
	RTAORenderer = this;
}

FRTAORenderer::~FRTAORenderer()
{
	RTAORenderer = nullptr;
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
	RT_BasePass->AddColorBuffer(EPF_RGBA32F, 1, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, 1, ERTC_COLOR1, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, 1, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	// Load default pipeline
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetPtr DebugMaterialAsset = TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TResourcePtr DebugMaterialResource = DebugMaterialAsset->GetResourcePtr();
	TMaterialPtr DebugMaterial = static_cast<TMaterial*>(DebugMaterialResource.get());
	FPipelinePtr DebugPipeline = DebugMaterial->PipelineResource;

	AB_RenderResult = RHI->CreateArgumentBuffer(1);
	{
		AB_RenderResult->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		//AB_Result->SetTexture(0, T_GBuffer[GBUFFER_COLOR]);
		RHI->UpdateHardwareResourceAB(AB_RenderResult, FSRender.GetFullScreenShader(), 0);
	}

	// Create GBuffer
	TTextureDesc TextureDesc;
	TextureDesc.Format = EPF_RGBA16F;// EPF_RGBA8;
	TextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;

#define CreateTextureResource(T) T=RHI->CreateTexture(TextureDesc); \
	T->SetTextureFlag(ETF_UAV, true); \
	T->SetResourceName(#T); \
	RHI->UpdateHardwareResourceTexture(T)

	TextureDesc.Width = RTWidth;
	TextureDesc.Height = RTHeight;
	CreateTextureResource(T_GBuffer[GBUFFER_COLOR]);
#undef CreateTextureResource

	AB_RtxResult = RHI->CreateArgumentBuffer(1);
	{
		AB_RtxResult->SetTexture(0, T_GBuffer[GBUFFER_COLOR]);
		RHI->UpdateHardwareResourceAB(AB_RtxResult, FSRender.GetFullScreenShader(), 0);
	}

	// For path tracer
	// Create RTX pipeline state object
	const TString RTAOPipeline = "RTX_AO.tasset";
	TAssetPtr RtxAOPipelineAsset = TAssetLibrary::Get()->LoadAsset(RTAOPipeline);
	TResourcePtr RtxAOPipelineResource = RtxAOPipelineAsset->GetResourcePtr();
	TRtxPipelinePtr RtxAOPipeline = static_cast<TRtxPipeline*>(RtxAOPipelineResource.get());
	RtxAOPSO = RtxAOPipeline->PipelineResource;

	// Create constant buffer
	UB_Pathtracer = ti_new FPathtracerUniform();
	UB_Pathtracer->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	// Create resource table
	ResourceTable = RHI->CreateRenderResourceTable(2, EHT_SHADER_RESOURCE);
	ResourceTable->PutRWTextureInTable(T_GBuffer[GBUFFER_COLOR], 0, UAV_RTAO);
	ResourceTable->PutTextureInTable(RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture, SRV_SCENE_POSITION);
	ResourceTable->PutTextureInTable(RT_BasePass->GetColorBuffer(ERTC_COLOR1).Texture, SRV_SCENE_NORMAL);
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

void FRTAORenderer::UpdateCamInfo(FScene* Scene)
{
	const FViewProjectionInfo& VPInfo = Scene->GetViewProjection();
	matrix4 VP = VPInfo.MatProj* VPInfo.MatView;
	matrix4 InvVP;
	VP.getInverse(InvVP);

	UB_Pathtracer->UniformBufferData[0].CamPos = FFloat4(VPInfo.CamPos.X, VPInfo.CamPos.Y, VPInfo.CamPos.Z, 1.f);
	UB_Pathtracer->UniformBufferData[0].ProjectionToWorld = InvVP;
	UB_Pathtracer->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
}

void FRTAORenderer::Render(FRHI* RHI, FScene* Scene)
{
	UpdateCamInfo(Scene);
	RHI->SetResourceStateTexture(RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture, RESOURCE_STATE_RENDER_TARGET, false);
	RHI->SetResourceStateTexture(RT_BasePass->GetColorBuffer(ERTC_COLOR1).Texture, RESOURCE_STATE_RENDER_TARGET, false);
	RHI->FlushResourceStateChange();
	RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass-Normal");
	DrawSceneTiles(RHI, Scene);

	vector3di TraceSize;
	TraceSize.X = RHI->GetViewport().Width;
	TraceSize.Y = RHI->GetViewport().Height;
	TraceSize.Z = 1;

	if (Scene->GetTLAS() != nullptr && Scene->GetTLAS()->AlreadyBuilt())
	{
		RHI->SetResourceStateTexture(RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(RT_BasePass->GetColorBuffer(ERTC_COLOR1).Texture, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateTexture(T_GBuffer[GBUFFER_COLOR], RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		//RHI->SetResourceStateAS(Scene->GetTLAS(), RESOURCE_STATE_RAYTRACING_AS);
		ResourceTable->PutTopLevelAccelerationStructureInTable(Scene->GetTLAS(), SRV_AS);

		RHI->SetRtxPipeline(RtxAOPSO);
		RHI->SetComputeResourceTable(0, ResourceTable);
		//RHI->SetComputeConstantBuffer(1, UB_Pathtracer->UniformBuffer);
		RHI->TraceRays(RtxAOPSO, TraceSize);
	}

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_RenderResult);
	if (!true)
	{
		RHI->SetResourceStateTexture(T_GBuffer[GBUFFER_COLOR], RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		FSRender.DrawFullScreenTexture(RHI, AB_RtxResult);
	}
}
