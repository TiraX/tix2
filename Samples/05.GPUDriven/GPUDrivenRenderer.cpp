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


inline vector3df ti_abs(const vector3df& x)
{
	vector3df Ret;
	Ret.X = ti_abs(x.X);
	Ret.Y = ti_abs(x.Y);
	Ret.Z = ti_abs(x.Z);
	return Ret;
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

	// Create frustum uniform buffer
	FrustumUniform = ti_new FCameraFrustumUniform;

	// Prepare compute cull tasks
	FScene * Scene = FRenderThread::Get()->GetRenderScene();
	//TileCullCS = ti_new FGPUTileFrustumCullCS();
	//TileCullCS->Finalize();
	//TileCullCS->PrepareResources(RHI);

	InstanceCullCS = ti_new FGPUInstanceFrustumCullCS();
	InstanceCullCS->Finalize();
	InstanceCullCS->PrepareResources(RHI);

	// Prepare copy visible command buffer tasks
	CopyVisibleCommandBuffer = ti_new FCopyVisibleTileCommandBuffer;
	CopyVisibleCommandBuffer->Finalize();
	CopyVisibleCommandBuffer->PrepareResources(RHI);
}

void FGPUDrivenRenderer::UpdateGPUCommandBuffer(FRHI* RHI, FScene * Scene)
{
	// Encode all draw command to GPU command buffer
	TI_TODO("Process opaque list for now, encode other list in future.");

	const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList(LIST_OPAQUE);
	const uint32 PrimsCount = (uint32)Primitives.size();
	const uint32 PrimsAdded = SceneMetaInfo->GetSceneStaticMeshAdded();
	if (PrimsAdded == 0)
	{
		return;
	}
	GPUCommandBuffer = RHI->CreateGPUCommandBuffer(GPUCommandSignature, PrimsAdded, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
#if defined (TIX_DEBUG)
	GPUCommandBuffer->SetResourceName("DrawListCB");
#endif
	// Add binding arguments
	GPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);

	// Add draw calls
	uint32 CommandIndex = 0;
	for (uint32 i = 0 ; i < PrimsCount ; ++ i)
	{
		FPrimitivePtr Primitive = Primitives[i];
		const vector2di& TilePos = Primitive->GetSceneTilePos();
		if (!SceneMetaInfo->IsTileVisible(TilePos))
		{
			continue;
		}

		FMeshBufferPtr MeshBuffer = Primitive->GetMeshBuffer();
		FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
		TI_ASSERT(MeshBuffer != nullptr && InstanceBuffer != nullptr);
		GPUCommandBuffer->EncodeSetMeshBuffer(CommandIndex, GPU_COMMAND_SET_MESH_BUFFER, MeshBuffer, InstanceBuffer);
		GPUCommandBuffer->EncodeSetDrawIndexed(CommandIndex,
			GPU_COMMAND_DRAW_INDEXED,
			MeshBuffer->GetIndicesCount(),
			Primitive->GetInstanceCount(),
			0, 
			0, 
			Primitive->GetInstanceOffset());
		++CommandIndex;
	}
	TI_ASSERT(GPUCommandBuffer->GetEncodedCommandsCount() <= PrimsAdded);
	RHI->UpdateHardwareResourceGPUCommandBuffer(GPUCommandBuffer);

	// Create empty GPU command buffer, gather visible draw commands
	ProcessedGPUCommandBuffer = RHI->CreateGPUCommandBuffer(GPUCommandSignature, PrimsCount, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
#if defined (TIX_DEBUG)
	ProcessedGPUCommandBuffer->SetResourceName("ProcessedCB");
#endif
	// Add binding arguments
	ProcessedGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
	RHI->UpdateHardwareResourceGPUCommandBuffer(ProcessedGPUCommandBuffer);
}

void FGPUDrivenRenderer::UpdateFrustumUniform(const SViewFrustum& InFrustum)
{
	Frustum = InFrustum;
	FrustumUniform->UniformBufferData[0].BBoxMin = FFloat4(InFrustum.BoundingBox.MinEdge.X, InFrustum.BoundingBox.MinEdge.Y, InFrustum.BoundingBox.MinEdge.Z, 1.f);
	FrustumUniform->UniformBufferData[0].BBoxMax = FFloat4(InFrustum.BoundingBox.MaxEdge.X, InFrustum.BoundingBox.MaxEdge.Y, InFrustum.BoundingBox.MaxEdge.Z, 1.f);
	for (int32 i = SViewFrustum::VF_FAR_PLANE ; i < SViewFrustum::VF_PLANE_COUNT ; ++ i)
	{
		FrustumUniform->UniformBufferData[0].Planes[i] = FFloat4(
			InFrustum.Planes[i].Normal.X, 
			InFrustum.Planes[i].Normal.Y, 
			InFrustum.Planes[i].Normal.Z, 
			InFrustum.Planes[i].D);
	}
	FrustumUniform->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
}

void FGPUDrivenRenderer::DrawGPUCommandBuffer(FRHI * RHI, FGPUCommandBufferPtr InGPUCommandBuffer)
{
	if (InGPUCommandBuffer != nullptr)
	{
		RHI->ExecuteGPUCommands(InGPUCommandBuffer);
	}
}

void FGPUDrivenRenderer::Render(FRHI* RHI, FScene* Scene)
{
	TI_TODO("Cull Scene tile in render thread. Then only collect visible scene tile resources");
	SceneMetaInfo->DoSceneTileCulling(Scene, Frustum);

	SceneMetaInfo->CollectSceneMetaInfos(Scene);
	SceneMetaInfo->CollectInstanceBuffers(Scene);

	if (Scene->HasSceneFlag(FScene::ScenePrimitivesDirty))
	{
		// Update GPU Command Buffer
		UpdateGPUCommandBuffer(RHI, Scene);
	}

	//bool test = false;
	//if (test)
	//{
	//	_LOG(Log, "instances loaded = %d\n", FStats::Stats.InstancesLoaded);
	//}

	if (GPUCommandBuffer != nullptr)
	{
		if (Scene->HasSceneFlag(FScene::ViewProjectionDirty))
		{
			// Add binding arguments
			GPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
			ProcessedGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
		}
		// Set meta data every frame.
		if (Scene->HasSceneFlag(FScene::ViewProjectionDirty) ||
			SceneMetaInfo->HasMetaFlag(FSceneMetaInfos::MetaFlag_SceneInstanceMetaDirty))
		{
			InstanceCullCS->UpdateComputeArguments(
				RHI,
				SceneMetaInfo->GetPrimitiveBBoxesUniform(),
				SceneMetaInfo->GetInstanceMetaUniform(),
				SceneMetaInfo->GetMergedInstanceBuffer(),
				FrustumUniform->UniformBuffer
			);
		}
		//if (Scene->HasSceneFlag(FScene::ScenePrimitivesDirty))
		//{
		//	// Update Copy command buffer params
		//	CopyVisibleCommandBuffer->UpdateComputeArguments(
		//		RHI,
		//		Scene,
		//		TileCullCS->GetVisibilityResult(),
		//		SceneMetaInfo->GetPrimitiveMetaUniform(),
		//		GPUCommandBuffer,
		//		ProcessedGPUCommandBuffer);
		//}

		{
			RHI->BeginComputeTask();
			// Do GPU tile frustum culling(Move to CPU tile culling)
			//TileCullCS->Run(RHI);

			// Do GPU Instance frustum culling
			InstanceCullCS->Run(RHI);

			// Copy visible tile command buffers
			//static bool CopyVisibleBuffer = true;
			//if (CopyVisibleBuffer)
			//	CopyVisibleCommandBuffer->Run(RHI);
			RHI->EndComputeTask();
		}


		{
			// Render Base Pass
			RHI->BeginRenderToRenderTarget(RT_BasePass, "BasePass");

			bool Indirect = false;
			bool DrawCulled = !false;
			if (!Indirect)
			{
				RenderDrawList(RHI, Scene, LIST_OPAQUE);
				//RenderDrawList(RHI, Scene, LIST_MASK);
			}
			else
			{
				DrawGPUCommandBuffer(RHI, DrawCulled ? ProcessedGPUCommandBuffer : GPUCommandBuffer);
			}
		}
	}

	RHI->BeginRenderToFrameBuffer();
	FSRender.DrawFullScreenTexture(RHI, AB_Result);

	SceneMetaInfo->ClearMetaFlags();
}
