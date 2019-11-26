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

	// Init GPU command buffer
	TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	CommandStructure.reserve(2);
	CommandStructure.push_back(GPU_COMMAND_SET_VERTEX_BUFFER);
	//CommandStructure.push_back(GPU_COMMAND_SET_INSTANCE_BUFFER);
	CommandStructure.push_back(GPU_COMMAND_SET_INDEX_BUFFER);
	CommandStructure.push_back(GPU_COMMAND_DRAW_INDEXED);

	GPUCommandSignature = RHI->CreateGPUCommandSignature(DebugPipeline, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(GPUCommandSignature);

	PreZGPUCommandSignature = RHI->CreateGPUCommandSignature(DepthOnlyPipeline, CommandStructure);
	RHI->UpdateHardwareResourceGPUCommandSig(PreZGPUCommandSignature);

	// Create frustum uniform buffer
	FrustumUniform = ti_new FCameraFrustumUniform;

	// Prepare compute cull tasks
	FScene * Scene = FRenderThread::Get()->GetRenderScene();

	InstanceFrustumCullCS = ti_new FGPUInstanceFrustumCullCS();
	InstanceFrustumCullCS->Finalize();
	InstanceFrustumCullCS->PrepareResources(RHI);

	// Prepare copy visible occluders & instances
	CopyVisibleOccluders = ti_new FCopyVisibleInstances;
	CopyVisibleOccluders->Finalize();
	CopyVisibleOccluders->PrepareResources(RHI);

	CopyVisibleInstances = ti_new FCopyVisibleInstances;
	CopyVisibleInstances->Finalize();
	CopyVisibleInstances->PrepareResources(RHI);

	// Down sample HiZ 
	HiZDownSample = ti_new FHiZDownSampleCS;
	HiZDownSample->Finalize();
	HiZDownSample->PrepareResources(RHI, vector2di(RTWidth, RTHeight), HiZTexture);

	// Instance occlusion cull
	InstanceOcclusionCullCS = ti_new FGPUInstanceOcclusionCullCS;
	InstanceOcclusionCullCS->Finalize();
	InstanceOcclusionCullCS->PrepareResources(RHI, vector2di(RTWidth, RTHeight), HiZTexture);
}

void FGPUDrivenRenderer::UpdateGPUCommandBuffer(FRHI* RHI, FScene * Scene)
{
	// Encode all draw command to GPU command buffer in Tile cull order.
	TI_TODO("Process opaque list for now, encode other list in future.");

	const uint32 PrimsAdded = SceneMetaInfo->GetScenePrimitivesAdded();
	if (PrimsAdded == 0)
	{
		return;
	}

	GPUCommandBuffer = RHI->CreateGPUCommandBuffer(GPUCommandSignature, PrimsAdded, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
	GPUCommandBuffer->SetResourceName("DrawListCB");
	// Add binding arguments
	GPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);

	PreZGPUCommandBuffer = RHI->CreateGPUCommandBuffer(PreZGPUCommandSignature, PrimsAdded, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
	PreZGPUCommandBuffer->SetResourceName("OccluderCB");
	// Add binding arguments
	PreZGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);

	// Add draw calls
	uint32 CommandIndex = 0;
	const TVector<vector2di>& SortedTilePositions = SceneMetaInfo->GetSortedTilePositions();
	const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
	for (const auto& TilePos : SortedTilePositions)
	{
		if (!SceneMetaInfo->IsTileVisible(TilePos))
		{
			continue;
		}
		FSceneTileResourcePtr TileRes = SceneTileResources.find(TilePos)->second;

		const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
		for (uint32 PIndex = 0; PIndex < (uint32)TilePrimitives.size(); ++PIndex)
		{
			FPrimitivePtr Primitive = TilePrimitives[PIndex];
			if (Primitive != nullptr)
			{
				FMeshBufferPtr MeshBuffer = Primitive->GetMeshBuffer();
				FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
				TI_ASSERT(MeshBuffer != nullptr && InstanceBuffer != nullptr);
				GPUCommandBuffer->EncodeSetVertexBuffer(CommandIndex, 0, MeshBuffer);
				//GPUCommandBuffer->EncodeSetInstanceBuffer(CommandIndex, GPU_COMMAND_SET_INSTANCE_BUFFER, InstanceBuffer);
				GPUCommandBuffer->EncodeSetIndexBuffer(CommandIndex, 1, MeshBuffer);
				GPUCommandBuffer->EncodeSetDrawIndexed(CommandIndex,
					2,
					MeshBuffer->GetIndicesCount(),
					Primitive->GetInstanceCount(),
					0,
					0,
					Primitive->GetGlobalInstanceOffset());

				// Encode occluders
				FMeshBufferPtr OccludeMesh = Primitive->GetOccluderMesh();
				if (OccludeMesh != nullptr)
				{
					PreZGPUCommandBuffer->EncodeSetVertexBuffer(CommandIndex, 0, OccludeMesh);
					PreZGPUCommandBuffer->EncodeSetIndexBuffer(CommandIndex, 1, OccludeMesh);
					PreZGPUCommandBuffer->EncodeSetDrawIndexed(CommandIndex,
						2,
						OccludeMesh->GetIndicesCount(),
						Primitive->GetInstanceCount(),
						0,
						0,
						Primitive->GetGlobalInstanceOffset());
				}
				else
				{
					PreZGPUCommandBuffer->EncodeEmptyCommand(CommandIndex);
				}

				++CommandIndex;
			}
		}
	}

	TI_ASSERT(GPUCommandBuffer->GetEncodedCommandsCount() <= PrimsAdded);
	RHI->UpdateHardwareResourceGPUCommandBuffer(GPUCommandBuffer);
	RHI->UpdateHardwareResourceGPUCommandBuffer(PreZGPUCommandBuffer);

	// Create empty GPU command buffer, gather visible draw commands, make it enough for all instances
	const uint32 TotalInstances = SceneMetaInfo->GetSceneInstancesAdded();
	ProcessedGPUCommandBuffer = RHI->CreateGPUCommandBuffer(GPUCommandSignature, TotalInstances, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
#if defined (TIX_DEBUG)
	ProcessedGPUCommandBuffer->SetResourceName("ProcessedCB");
#endif
	// Add binding arguments
	ProcessedGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
	RHI->UpdateHardwareResourceGPUCommandBuffer(ProcessedGPUCommandBuffer);


	ProcessedPreZGPUCommandBuffer = RHI->CreateGPUCommandBuffer(PreZGPUCommandSignature, TotalInstances, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE | UB_FLAG_COMPUTE_WRITABLE | UB_FLAG_COMPUTE_WITH_COUNTER);
#if defined (TIX_DEBUG)
	ProcessedPreZGPUCommandBuffer->SetResourceName("ProcessedOccluderCB");
#endif
	// Add binding arguments
	ProcessedPreZGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
	RHI->UpdateHardwareResourceGPUCommandBuffer(ProcessedPreZGPUCommandBuffer);
}

void FGPUDrivenRenderer::SimluateCopyVisibleInstances(FRHI* RHI, FScene * Scene)
{
	const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList(LIST_OPAQUE);
	const uint32 PrimsCount = (uint32)Primitives.size();
	const uint32 PrimsAdded = SceneMetaInfo->GetScenePrimitivesAdded();
	const uint32 TotalInstances = SceneMetaInfo->GetSceneInstancesAdded();
	if (PrimsAdded == 0)
	{
		return;
	}

	GPUCommandBufferTest = RHI->CreateGPUCommandBuffer(GPUCommandSignature, TotalInstances, UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
#if defined (TIX_DEBUG)
	GPUCommandBufferTest->SetResourceName("DrawListCBTest");
#endif
	GPUCommandBufferTest->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);

	// Copy instance as single draw call in gpu command buffer
	typedef struct IndirectCommand
	{
		FUInt4 VBView;
		FUInt4 IBView;

		FUInt4 DrawArg0;
		uint32 DrawArg1;
	} IndirectCommand;
	FSceneInstanceMetaInfoPtr InstanceMetaInfo = SceneMetaInfo->GetInstanceMetaUniform();
	const uint32 CommandStride = GPUCommandSignature->GetCommandStrideInBytes();
	TI_ASSERT(CommandStride == sizeof(IndirectCommand));
	uint32 CommandTestIndex = 0;
	for (uint32 Ins = 0 ; Ins < TotalInstances ; ++ Ins)
	{
		if (InstanceMetaInfo->UniformBufferData[Ins].Info.W > 0)
		{
			IndirectCommand ICB;
			uint32 CommandIndex = InstanceMetaInfo->UniformBufferData[Ins].Info.Y;
			memcpy(&ICB, GPUCommandBuffer->GetCommandData(CommandIndex), CommandStride);

			ICB.DrawArg0.Y = 1;
			ICB.DrawArg1 = Ins;

			GPUCommandBufferTest->SetCommandData(CommandTestIndex, &ICB, CommandStride);
			++CommandTestIndex;
		}
	}
	TI_ASSERT(GPUCommandBufferTest->GetEncodedCommandsCount() <= TotalInstances);
	RHI->UpdateHardwareResourceGPUCommandBuffer(GPUCommandBufferTest);
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
		// Set merged instance buffer
		RHI->SetResourceStateCB(InGPUCommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);
		RHI->SetInstanceBufferAtSlot(1, SceneMetaInfo->GetMergedInstanceBuffer());
		RHI->ExecuteGPUCommands(InGPUCommandBuffer);
	}
}

void FGPUDrivenRenderer::DrawSceneTiles(FRHI* RHI, FScene * Scene)
{
	const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
	for (auto& TileIter : SceneTileResources)
	{
		const vector2di& TilePos = TileIter.first;
		FSceneTileResourcePtr TileRes = TileIter.second;

		if (!SceneMetaInfo->IsTileVisible(TilePos))
		{
			continue;
		}

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

void FGPUDrivenRenderer::Render(FRHI* RHI, FScene* Scene)
{
	static bool bPerformGPUCulling = true;
	static bool bOcclusionCull = true;
	static bool Indirect = !false;
	static bool DrawCulled = !false;

	if (bPerformGPUCulling)
	{
		SceneMetaInfo->DoSceneTileCulling(Scene, Frustum);

		SceneMetaInfo->CollectSceneMetaInfos(Scene);
		SceneMetaInfo->CollectInstanceBuffers(Scene);
		SceneMetaInfo->CollectClusterMetaBuffers(Scene);
	}

	if (bPerformGPUCulling && 
		SceneMetaInfo->HasMetaFlag(FSceneMetaInfos::MetaFlag_SceneTileMetaDirty | FSceneMetaInfos::MetaFlag_ScenePrimitiveMetaDirty))
	{
		_LOG(Log, "Update gpu command buffer.\n");
		// Update GPU Command Buffer
		UpdateGPUCommandBuffer(RHI, Scene);
		//SimluateCopyVisibleInstances(RHI, Scene);
	}

	if (GPUCommandBuffer != nullptr && SceneMetaInfo->IsPrimitiveDataReady())
	{
		if (Scene->HasSceneFlag(FScene::ViewProjectionDirty))
		{
			_LOG(Log, "Update gpu command buffer args.\n");
			// Add binding arguments
			GPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
			PreZGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
			ProcessedGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
			ProcessedPreZGPUCommandBuffer->AddVSPublicArgument(0, Scene->GetViewUniformBuffer()->UniformBuffer);
		}
		// Set meta data every frame.
		if (Scene->HasSceneFlag(FScene::ViewProjectionDirty) ||
			SceneMetaInfo->HasMetaFlag(FSceneMetaInfos::MetaFlag_SceneInstanceMetaDirty))
		{
			if (bPerformGPUCulling)
			{
				_LOG(Log, "Update instance cull CS args.\n");
				InstanceFrustumCullCS->UpdateComputeArguments(
					RHI,
					SceneMetaInfo->GetPrimitiveBBoxesUniform()->UniformBuffer,
					SceneMetaInfo->GetInstanceMetaUniform()->UniformBuffer,
					SceneMetaInfo->GetMergedInstanceBuffer(),
					FrustumUniform->UniformBuffer,
					SceneMetaInfo->GetSceneInstancesIntersected()
				);
				_LOG(Log, "Instances : %d, Intersected : %d.\n", SceneMetaInfo->GetSceneInstancesAdded(), SceneMetaInfo->GetSceneInstancesIntersected());
				const FViewProjectionInfo&  ViewProjection = Scene->GetViewProjection();
				InstanceOcclusionCullCS->UpdateComputeArguments(
					RHI,
					ViewProjection.MatProj * ViewProjection.MatView,
					SceneMetaInfo->GetPrimitiveBBoxesUniform()->UniformBuffer,
					SceneMetaInfo->GetInstanceMetaUniform()->UniformBuffer,
					SceneMetaInfo->GetMergedInstanceBuffer(),
					InstanceFrustumCullCS->GetVisibleResult(),
					SceneMetaInfo->GetSceneInstancesAdded());
			}
		}
		if (SceneMetaInfo->HasMetaFlag(FSceneMetaInfos::MetaFlag_SceneInstanceMetaDirty))
		{
			if (bPerformGPUCulling)
			{
				_LOG(Log, "Update copy visible instances args.\n");
				CopyVisibleOccluders->UpdateComputeArguments(
					RHI,
					SceneMetaInfo,
					InstanceFrustumCullCS->GetVisibleResult(),
					SceneMetaInfo->GetInstanceMetaUniform()->UniformBuffer,
					PreZGPUCommandBuffer,
					ProcessedPreZGPUCommandBuffer
				);
				CopyVisibleInstances->UpdateComputeArguments(
					RHI,
					SceneMetaInfo,
					bOcclusionCull ? InstanceOcclusionCullCS->GetVisibleResult() : InstanceFrustumCullCS->GetVisibleResult(),
					SceneMetaInfo->GetInstanceMetaUniform()->UniformBuffer,
					GPUCommandBuffer,
					ProcessedGPUCommandBuffer
				);
			}
		}

		if (bPerformGPUCulling)
		{
			RHI->BeginComputeTask();

			// Do GPU Instance frustum culling
			{
				RHI->BeginEvent("Instance Frustum Cull");
				InstanceFrustumCullCS->Run(RHI);
				RHI->EndEvent();
			}

			// Copy pre-z occluders only.
			if (bOcclusionCull)
			{
				RHI->BeginEvent("Copy Visible Occluders");
				CopyVisibleOccluders->Run(RHI);
				RHI->EndEvent();
			}

			RHI->EndComputeTask();
		}

		if (bOcclusionCull)
		{
			// PreZ Pass
			RHI->BeginRenderToRenderTarget(RT_DepthOnly, 0, "PreZ");
			if (!Indirect)
			{
				// Set depth only PSO
				RHI->SetGraphicsPipeline(DepthOnlyPipeline);

				// Render occlude mesh
				const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();
				for (auto& TileIter : SceneTileResources)
				{
					const vector2di& TilePos = TileIter.first;
					FSceneTileResourcePtr TileRes = TileIter.second;

					if (!SceneMetaInfo->IsTileVisible(TilePos))
					{
						continue;
					}

					const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
					for (uint32 PIndex = 0; PIndex < (uint32)TilePrimitives.size(); ++PIndex)
					{
						FPrimitivePtr Primitive = TilePrimitives[PIndex];

						if (Primitive != nullptr && Primitive->GetOccluderMesh() != nullptr)
						{
							FInstanceBufferPtr InstanceBuffer = Primitive->GetInstanceBuffer();
							RHI->SetMeshBuffer(Primitive->GetOccluderMesh(), InstanceBuffer);
							ApplyShaderParameter(RHI, Scene, Primitive);

							RHI->DrawPrimitiveIndexedInstanced(
								Primitive->GetOccluderMesh(),
								InstanceBuffer == nullptr ? 1 : Primitive->GetInstanceCount(),
								Primitive->GetInstanceOffset());
						}
					}
				}
			}
			else
			{

				if (ProcessedPreZGPUCommandBuffer != nullptr)
				{
					// Set merged instance buffer
					RHI->SetResourceStateCB(ProcessedPreZGPUCommandBuffer, RESOURCE_STATE_INDIRECT_ARGUMENT);
					RHI->SetInstanceBufferAtSlot(1, SceneMetaInfo->GetMergedInstanceBuffer());
					RHI->ExecuteGPUCommands(ProcessedPreZGPUCommandBuffer);
				}
			}
			RHI->CopyTextureRegion(HiZTexture, recti(0, 0, HiZTexture->GetWidth(), HiZTexture->GetHeight()), 0, RT_DepthOnly->GetDepthStencilBuffer().Texture, 0);
		}

		{
			if (bPerformGPUCulling)
			{
				RHI->BeginComputeTask();
				if (bOcclusionCull)
				{
					// Occlusion Cull
					// Down Sample Depth
					RHI->BeginEvent("Down Sample HiZ");
					for (uint32 i = 1; i < FHiZDownSampleCS::HiZLevels; ++i)
					{
						HiZDownSample->UpdateComputeArguments(RHI, i);
						HiZDownSample->Run(RHI);
					}
					RHI->EndEvent();

					// Cull Occluded Instances
					RHI->BeginEvent("Cull occluded Instances");
					InstanceOcclusionCullCS->Run(RHI);
					RHI->EndEvent();
				}

				// Copy visible instances only
				static bool bCopyVisibleInstances = true;
				if (bCopyVisibleInstances)
				{
					RHI->BeginEvent("Copy Visible Instances");
					CopyVisibleInstances->Run(RHI);
					RHI->EndEvent();
				}

				RHI->EndComputeTask();
			}
		}

		{
			// Render Base Pass
			RHI->BeginRenderToRenderTarget(RT_BasePass, 0, "BasePass");

			if (!Indirect)
			{
				DrawSceneTiles(RHI, Scene);
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
