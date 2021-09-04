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
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, 1, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, 1, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	// Load default pipeline
	const TString DefaultMaterial = "M_Debug.tasset";
	TAssetPtr DebugMaterialAsset = TAssetLibrary::Get()->LoadAsset(DefaultMaterial);
	TResourcePtr DebugMaterialResource = DebugMaterialAsset->GetResourcePtr();
	TMaterialPtr DebugMaterial = static_cast<TMaterial*>(DebugMaterialResource.get());
	FPipelinePtr DebugPipeline = DebugMaterial->PipelineResource;

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

	AB_Result = RHI->CreateArgumentBuffer(1);
	{
		AB_Result->SetTexture(0, RT_BasePass->GetColorBuffer(ERTC_COLOR0).Texture);
		//AB_Result->SetTexture(0, T_GBuffer[GBUFFER_COLOR]);
		RHI->UpdateHardwareResourceAB(AB_Result, FSRender.GetFullScreenShader(), 0);
	}


	// For path tracer
	// Create RTX pipeline state object
	const TString PathtracerPipeline = "RTX_Pathtracer.tasset";
	TAssetPtr RtxPipelineAsset = TAssetLibrary::Get()->LoadAsset(PathtracerPipeline);
	TResourcePtr RtxPipelineResource = RtxPipelineAsset->GetResourcePtr();
	TRtxPipelinePtr RtxPipeline = static_cast<TRtxPipeline*>(RtxPipelineResource.get());
	RtxPSO = RtxPipeline->PipelineResource;

	// Create constant buffer
	UB_Pathtracer = ti_new FPathtracerUniform();
	UB_Pathtracer->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	// Create resource table
	ResourceTable = RHI->CreateRenderResourceTable(2, EHT_SHADER_RESOURCE);
	ResourceTable->PutRWTextureInTable(T_GBuffer[GBUFFER_COLOR], 0, 0);
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

void FRTAORenderer::UpdateCamInfo(const vector3df& Pos, const vector3df& Dir, const vector3df& Hor, const vector3df& Ver)
{
	UB_Pathtracer->UniformBufferData[0].CamPos = FFloat4(Pos.X, Pos.Y, Pos.Z, 1.f);
	UB_Pathtracer->UniformBufferData[0].CamU = FFloat4(Hor.X, Hor.Y, Hor.Z, 1.f);
	UB_Pathtracer->UniformBufferData[0].CamV = FFloat4(Ver.X, Ver.Y, Ver.Z, 1.f);
	UB_Pathtracer->UniformBufferData[0].CamW = FFloat4(Dir.X, Dir.Y, Dir.Z, 1.f);
	UB_Pathtracer->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
}

void FRTAORenderer::Render(FRHI* RHI, FScene* Scene)
{
	//RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");
	//DrawSceneTiles(RHI, Scene);

	vector3di TraceSize;
	TraceSize.X = RHI->GetViewport().Width;
	TraceSize.Y = RHI->GetViewport().Height;
	TraceSize.Z = 1;

	if (Scene->GetTLAS() != nullptr && Scene->GetTLAS()->AlreadyBuilt())
	{
		ResourceTable->PutTopLevelAccelerationStructureInTable(Scene->GetTLAS(), 1);

		RHI->SetRtxPipeline(RtxPSO);
		RHI->SetComputeResourceTable(0, ResourceTable);
		RHI->SetComputeConstantBuffer(1, UB_Pathtracer->UniformBuffer);
		RHI->TraceRays(RtxPSO, TraceSize);
	}

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
