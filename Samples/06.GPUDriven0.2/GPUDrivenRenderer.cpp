/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GPUDrivenRenderer.h"
#include "SceneMetaInfos.h"

FGPUDrivenRenderer * GPUDrivenRenderer = nullptr;
FGPUDrivenRenderer * FGPUDrivenRenderer::Get()
{
	return GPUDrivenRenderer;
}

FGPUDrivenRenderer::FGPUDrivenRenderer()
	: SceneMetaInfo(nullptr)
{
	GPUDrivenRenderer = this;
	SceneMetaInfo = ti_new FSceneMetaInfos();
}

FGPUDrivenRenderer::~FGPUDrivenRenderer()
{
	GPUDrivenRenderer = nullptr;
	ti_delete SceneMetaInfo;
}

void FGPUDrivenRenderer::InitRenderFrame(FScene* Scene)
{
	// Prepare frame view uniform buffer
	Scene->InitRenderFrame();
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
	RT_BasePass->SetResourceName("RT_BasePass");
	RT_BasePass->AddColorBuffer(EPF_RGBA16F, 1, ERTC_COLOR0, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_BasePass->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8, 1, ERT_LOAD_CLEAR, ERT_STORE_DONTCARE);
	RT_BasePass->Compile();

	// Setup depth only render target
	RT_DepthOnly = FRenderTarget::Create(RTWidth, RTHeight);
	RT_DepthOnly->SetResourceName("RT_DepthOnly");
	RT_DepthOnly->AddDepthStencilBuffer(EPF_DEPTH32, 1, ERT_LOAD_CLEAR, ERT_STORE_STORE);
	RT_DepthOnly->Compile();

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
	FPipelinePtr DebugPipeline = DebugMaterial->PipelineResource;

	// Load Pre-Z pipeline.
	const TString DepthOnlyMaterialName = "M_DepthOnly.tasset";
	TAssetPtr DepthOnlyMaterialAsset = TAssetLibrary::Get()->LoadAsset(DepthOnlyMaterialName);
	TResourcePtr DepthOnlyMaterialResource = DepthOnlyMaterialAsset->GetResourcePtr();
	TMaterialPtr DepthOnlyMaterial = static_cast<TMaterial*>(DepthOnlyMaterialResource.get());
	DepthOnlyPipeline = DepthOnlyMaterial->PipelineResource;

	TTextureDesc HiZTextureDesc;
	HiZTextureDesc.Type = ETT_TEXTURE_2D;
	HiZTextureDesc.Format = EPF_R32F;
	HiZTextureDesc.Width = RTWidth;
	HiZTextureDesc.Height = RTHeight;
	HiZTextureDesc.AddressMode = ETC_CLAMP_TO_EDGE;
	HiZTextureDesc.SRGB = 0;
	HiZTextureDesc.Mips = FHiZDownSampleCS::HiZLevels;

	HiZTexture = RHI->CreateTexture(HiZTextureDesc);
	HiZTexture->SetTextureFlag(ETF_UAV, true);
	HiZTexture->SetResourceName("HiZTextures");
	RHI->UpdateHardwareResourceTexture(HiZTexture);

	// Init GPU command buffer
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.resize(1);
	CommandStructure[0] = GPU_COMMAND_DRAW_INDEXED;

	GPUCommandSignature = RHI->CreateGPUCommandSignature(DebugPipeline, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(GPUCommandSignature);

	GPUOccludeCommandSignature = RHI->CreateGPUCommandSignature(DepthOnlyPipeline, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(GPUOccludeCommandSignature);

	// Create frustum uniform buffer
	FrustumUniform = ti_new FCameraFrustumUniform;

	// Prepare compute cull tasks
	FScene * Scene = FRenderThread::Get()->GetRenderScene();

	InstanceFrustumCullCS = ti_new FInstanceFrustumCullCS();
	InstanceFrustumCullCS->Finalize();
	InstanceFrustumCullCS->PrepareResources(RHI);

	// Down sample HiZ 
	HiZDownSample = ti_new FHiZDownSampleCS;
	HiZDownSample->Finalize();
	HiZDownSample->PrepareResources(RHI, vector2di(RTWidth, RTHeight), HiZTexture);
}

void FGPUDrivenRenderer::UpdateFrustumUniform(const SViewFrustum& InFrustum)
{
	Frustum = InFrustum;
	FrustumUniform->UniformBufferData[0].BBoxMin = FFloat4(InFrustum.BoundingBox.MinEdge.X, InFrustum.BoundingBox.MinEdge.Y, InFrustum.BoundingBox.MinEdge.Z, 1.f);
	FrustumUniform->UniformBufferData[0].BBoxMax = FFloat4(InFrustum.BoundingBox.MaxEdge.X, InFrustum.BoundingBox.MaxEdge.Y, InFrustum.BoundingBox.MaxEdge.Z, 1.f);
	for (int32 i = SViewFrustum::VF_FAR_PLANE; i < SViewFrustum::VF_PLANE_COUNT; ++i)
	{
		FrustumUniform->UniformBufferData[0].Planes[i] = FFloat4(
			InFrustum.Planes[i].Normal.X,
			InFrustum.Planes[i].Normal.Y,
			InFrustum.Planes[i].Normal.Z,
			InFrustum.Planes[i].D);
	}
	FrustumUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
}

void FGPUDrivenRenderer::TestDrawSceneIndirectCommandBuffer(
	FRHI * RHI,
	FScene * Scene,
	FMeshBufferPtr MeshBuffer,
	FInstanceBufferPtr InstanceBuffer,
	FGPUCommandSignaturePtr CommandSignature,
	FGPUCommandBufferPtr CommandBuffer)
{
	if (CommandBuffer != nullptr)
	{
		RHI->SetResourceStateCB(CommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);
		RHI->SetMeshBuffer(MeshBuffer, InstanceBuffer);
		RHI->SetGraphicsPipeline(CommandSignature->GetPipeline());
		RHI->SetUniformBuffer(ESS_VERTEX_SHADER, 0, Scene->GetViewUniformBuffer()->UniformBuffer);
		RHI->ExecuteGPUDrawCommands(CommandBuffer);
	}
}

void FGPUDrivenRenderer::Render(FRHI* RHI, FScene* Scene)
{
	static bool bGPUCullEnabled = true;
	// Merge all instances and meshes into a big one
	SceneMetaInfo->PrepareSceneResources(RHI, Scene, GPUCommandSignature);

	bool bGPUCull = bGPUCullEnabled && SceneMetaInfo->IsInited();
	TI_TODO("Use depth from last frame. Think carefully about it. NOT a clear solution yet.");

	// Prepare compute parameters
	if (bGPUCull)
	{
		InstanceFrustumCullCS->UpdataComputeParams(
			RHI,
			FrustumUniform->UniformBuffer,
			SceneMetaInfo->GetPrimitiveBBoxesUniform()->UniformBuffer,
			SceneMetaInfo->GetInstanceMetaInfoUniform()->UniformBuffer,
			SceneMetaInfo->GetMergedInstanceBuffer(),
			GPUOccludeCommandSignature,
			SceneMetaInfo->GetGPUOccludeCommandBuffer()
		);
	}

	// Frustum cull instances before preZ
	if (bGPUCull)
	{
		RHI->BeginComputeTask();
		RHI->BeginEvent("Frustum Instance Culling");
		InstanceFrustumCullCS->Run(RHI);
		RHI->EndEvent();
		RHI->EndComputeTask();
	}

	// Render preZ depth
	if (bGPUCull)
	{
		RHI->BeginRenderToRenderTarget(RT_DepthOnly, 0, "PreZ");
		TestDrawSceneIndirectCommandBuffer(RHI,
			Scene,
			SceneMetaInfo->GetMergedOccludeMeshBuffer(),
			InstanceFrustumCullCS->GetCompactInstanceBuffer(),
			GPUOccludeCommandSignature,
			InstanceFrustumCullCS->GetCulledDrawCommandBuffer());
		RHI->CopyTextureRegion(HiZTexture, recti(0, 0, HiZTexture->GetWidth(), HiZTexture->GetHeight()), 0, RT_DepthOnly->GetDepthStencilBuffer().Texture, 0);
	}

	if (bGPUCull)
	{
		RHI->BeginComputeTask();
		// Down Sample Depth
		{
			RHI->BeginEvent("Down Sample HiZ");
			for (uint32 i = 1; i < FHiZDownSampleCS::HiZLevels; ++i)
			{
				HiZDownSample->UpdateComputeArguments(RHI, i);
				HiZDownSample->Run(RHI);
			}
			RHI->EndEvent();
		}
		RHI->EndComputeTask();
	}

	// Option1: Instance occlusion cull / Cluster cull / Triangle cull

	// Option2: Cluster cull / Triangle cull

	{
		// Do test
		RHI->BeginRenderToRenderTarget(RT_BasePass, 0, "BasePass");
		//TestDrawSceneIndirectCommandBuffer(RHI, Scene, SceneMetaInfo->GetMergedSceneMeshBuffer(), InstanceFrustumCullCS->GetCompactInstanceBuffer(), InstanceFrustumCullCS->GetCulledDrawCommandBuffer());
		//TestDrawSceneIndirectCommandBuffer(RHI, Scene, SceneMetaInfo->GetMergedSceneMeshBuffer(), SceneMetaInfo->GetMergedInstanceBuffer(), SceneMetaInfo->GetGPUCommandBuffer());
	}

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);
}
