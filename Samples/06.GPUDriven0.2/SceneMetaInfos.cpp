/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SceneMetaInfos.h"

FSceneMetaInfos::FSceneMetaInfos()
	: Inited(false)
{
}

FSceneMetaInfos::~FSceneMetaInfos()
{
}

struct FDrawInstanceArgument
{
	uint32 IndexCountPerInstance;
	uint32 InstanceCount;
	uint32 StartIndexLocation;
	uint32 BaseVertexLocation;
	uint32 StartInstanceLocation;
};

void FSceneMetaInfos::PrepareSceneResources(FRHI* RHI, FScene * Scene, FGPUCommandSignaturePtr CommandSignature)
{
	// Re-collect instances and meshes
	if (Scene->HasSceneFlag(FScene::ScenePrimitivesDirty))
	{
		const THMap<vector2di, FSceneTileResourcePtr>& SceneTileResources = Scene->GetSceneTiles();

		uint32 TotalInstances = 0;
		uint32 TotalLoadedMeshes = 0;
		uint32 TotalLoadedOccludes = 0;
		uint32 SceneMeshesVertexCount = 0;
		uint32 SceneMeshIndexCount = 0;
		uint32 SceneOccludeVertexCount = 0;
		uint32 SceneOccludeIndexCount = 0;
		uint32 VBFormat = 0;
		for (const auto& T : SceneTileResources)
		{
			FSceneTileResourcePtr TileRes = T.second;

			// Stat total instances count
			TotalInstances += TileRes->GetInstanceBuffer()->GetInstancesCount();

			// Stat total meshes count, vertex and index count
			const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
			for (auto Prim : TileRes->GetPrimitives())
			{
				if (Prim != nullptr)
				{
					FMeshBufferPtr MeshBuffer = Prim->GetMeshBuffer();
					SceneMeshesVertexCount += MeshBuffer->GetVerticesCount();
					SceneMeshIndexCount += MeshBuffer->GetIndicesCount();
					TI_ASSERT(MeshBuffer->GetIndexType() == EIT_32BIT);	// Compute shader can only access 32bit aligned data
					TI_ASSERT(VBFormat == 0 || VBFormat == MeshBuffer->GetVSFormat());
					VBFormat = MeshBuffer->GetVSFormat();

					FMeshBufferPtr OccludeMeshBuffer = Prim->GetOccluderMesh();
					TI_ASSERT(OccludeMeshBuffer != nullptr);	// OccludeMeshBuffer can be nullptr ???
					if (OccludeMeshBuffer != nullptr)
					{
						SceneOccludeVertexCount += OccludeMeshBuffer->GetVerticesCount();
						SceneOccludeIndexCount += OccludeMeshBuffer->GetIndicesCount();
						TI_ASSERT(OccludeMeshBuffer->GetVSFormat() == EVSSEG_POSITION);
						TI_ASSERT(OccludeMeshBuffer->GetIndexType() == EIT_32BIT);	// Compute shader can only access 32bit aligned data
						++TotalLoadedOccludes;
					}

					++TotalLoadedMeshes;
				}
			}
			TI_ASSERT(TotalLoadedOccludes == TotalLoadedMeshes);
			TI_ASSERT(SceneMeshesVertexCount > 0 && SceneMeshIndexCount > 0);
		}

		TI_ASSERT(TotalInstances > 0 && TotalLoadedMeshes > 0);

		// Allocate space for draw arguments
		TVector<FDrawInstanceArgument> DrawArguments, OccludeDrawArguments;
		DrawArguments.resize(TotalLoadedMeshes);
		OccludeDrawArguments.resize(TotalLoadedOccludes);

		// Merge meshes and instances
		{
			// Create MergedMeshBuffer resource
			const uint32 VertexStride = TMeshBuffer::GetStrideFromFormat(VBFormat);
			MergedMeshBuffer = RHI->CreateEmptyMeshBuffer(EPT_TRIANGLELIST, VBFormat, SceneMeshesVertexCount, EIT_32BIT, SceneMeshIndexCount);
			MergedMeshBuffer->SetResourceName("MergedMeshBuffer");
			RHI->UpdateHardwareResourceMesh(
				MergedMeshBuffer, 
				SceneMeshesVertexCount * VertexStride, 
				VertexStride, 
				SceneMeshIndexCount * sizeof(uint32), 
				EIT_32BIT, 
				"MergedMeshBuffer"
			);

			// Create MergedOccludeMeshBuffer resource
			MergedOccludeMeshBuffer = RHI->CreateEmptyMeshBuffer(EPT_TRIANGLELIST, EVSSEG_POSITION, SceneOccludeVertexCount, EIT_32BIT, SceneOccludeIndexCount);
			MergedOccludeMeshBuffer->SetResourceName("MergedOccludeMeshBuffer");
			RHI->UpdateHardwareResourceMesh(
				MergedOccludeMeshBuffer, 
				SceneOccludeVertexCount * sizeof(vector3df), 
				sizeof(vector3df), 
				SceneOccludeIndexCount * sizeof(uint32), 
				EIT_32BIT, 
				"MergedOccludeMeshBuffer"
			);

			// Create MergedInstanceBuffer resource
			MergedInstanceBuffer = RHI->CreateEmptyInstanceBuffer(TotalInstances, TInstanceBuffer::InstanceStride);
			MergedInstanceBuffer->SetResourceName("MergedInstanceBuffer");
			RHI->UpdateHardwareResourceIB(MergedInstanceBuffer, nullptr);

			// Create Primitive BBox UniformBuffer resource
			PrimitiveBBoxesUniform = ti_new FScenePrimitiveBBoxes(TotalLoadedMeshes);

			// Create instance meta info UniformBuffer resource
			InstanceMetaInfoUniform = ti_new FSceneInstanceMetaInfo(TotalInstances);

			uint32 SceneVBOffset = 0, SceneIBOffset = 0;
			uint32 OccludeVBOffset = 0, OccludeIBOffset = 0;
			uint32 PrimIndex = 0;
			uint32 InstanceDstOffset = 0;
			for (const auto& T : SceneTileResources)
			{
				FSceneTileResourcePtr TileRes = T.second;

				// Merge meshes and collect primitive BBox
				{
					const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
					TI_TODO("Collected mesh buffer may be duplicated. Remove duplicated mesh. Copy a unique ONE.");

					// Copy mesh vertex and index to MergedMeshBuffer 
					// And record draw arguments
					for (auto Prim : TileRes->GetPrimitives())
					{
						if (Prim != nullptr)
						{
							// Scene mesh buffer
							FMeshBufferPtr MeshBuffer = Prim->GetMeshBuffer();
							RHI->SetResourceStateMB(MeshBuffer, RESOURCE_STATE_COPY_SOURCE);
							RHI->CopyBufferRegion(
								MergedMeshBuffer,
								SceneVBOffset,
								SceneIBOffset,
								MeshBuffer,
								0,
								MeshBuffer->GetVerticesCount() * VertexStride,
								0,
								MeshBuffer->GetIndicesCount() * sizeof(uint32)
							);

							// Remember draw arguments
							FDrawInstanceArgument& DrawArg = DrawArguments[PrimIndex];
							DrawArg.IndexCountPerInstance = MeshBuffer->GetIndicesCount();
							DrawArg.InstanceCount = Prim->GetInstanceCount();
							DrawArg.StartIndexLocation = SceneIBOffset / sizeof(uint32);
							DrawArg.BaseVertexLocation = SceneVBOffset / VertexStride;
							DrawArg.StartInstanceLocation = InstanceDstOffset + Prim->GetInstanceOffset();

							// Scene occlude mesh buffer
							FMeshBufferPtr OccludeMeshBuffer = Prim->GetOccluderMesh();
							TI_ASSERT(OccludeMeshBuffer != nullptr); // can be nullptr ???
							if (OccludeMeshBuffer != nullptr)
							{
								RHI->SetResourceStateMB(OccludeMeshBuffer, RESOURCE_STATE_COPY_SOURCE);
								RHI->CopyBufferRegion(
									MergedOccludeMeshBuffer,
									OccludeVBOffset,
									OccludeIBOffset,
									OccludeMeshBuffer,
									0,
									OccludeMeshBuffer->GetVerticesCount() * sizeof(vector3df),
									0,
									OccludeMeshBuffer->GetIndicesCount() * sizeof(uint32)
								);

								// Remember draw arguments
								FDrawInstanceArgument& OccludeDrawArg = OccludeDrawArguments[PrimIndex];
								OccludeDrawArg.IndexCountPerInstance = OccludeMeshBuffer->GetIndicesCount();
								OccludeDrawArg.InstanceCount = Prim->GetInstanceCount();
								OccludeDrawArg.StartIndexLocation = OccludeIBOffset / sizeof(uint32);
								OccludeDrawArg.BaseVertexLocation = OccludeVBOffset / sizeof(vector3df);
								OccludeDrawArg.StartInstanceLocation = InstanceDstOffset + Prim->GetInstanceOffset();
							}

							// Fill primitive bbox uniform buffer data
							const aabbox3df& BBox = Prim->GetBBox();
							PrimitiveBBoxesUniform->UniformBufferData[PrimIndex].MinEdge = FFloat4(BBox.MinEdge.X, BBox.MinEdge.Y, BBox.MinEdge.Z, 0.f);
							PrimitiveBBoxesUniform->UniformBufferData[PrimIndex].MaxEdge = FFloat4(BBox.MaxEdge.X, BBox.MaxEdge.Y, BBox.MaxEdge.Z, 0.f);

							// Fill instance meta info uniform buffer data
							for (uint32 Ins = DrawArg.StartInstanceLocation ; Ins < DrawArg.StartInstanceLocation + DrawArg.InstanceCount ; ++ Ins)
							{
								InstanceMetaInfoUniform->UniformBufferData[Ins].Info.X = PrimIndex;
								InstanceMetaInfoUniform->UniformBufferData[Ins].Info.W = 1;	// Mark as Loaded
							}

							// Remember offsets
							SceneVBOffset += MeshBuffer->GetVerticesCount() * VertexStride;
							SceneIBOffset += MeshBuffer->GetIndicesCount() * sizeof(uint32);

							++PrimIndex;
						}
					}
				}

				// Merge tile instances
				{
					FInstanceBufferPtr TileInstances = TileRes->GetInstanceBuffer();

					RHI->CopyBufferRegion(MergedInstanceBuffer, InstanceDstOffset, TileInstances, 0, TileInstances->GetInstancesCount());
					InstanceDstOffset += TileInstances->GetInstancesCount();
				}
			}
			TI_ASSERT(InstanceDstOffset == TotalInstances);
			TI_ASSERT(PrimIndex == TotalLoadedMeshes);

			PrimitiveBBoxesUniform->InitUniformBuffer();
			InstanceMetaInfoUniform->InitUniformBuffer();
		}

		// Create scene indirect draw command buffer
		{
			// Scene indirect draw command buffer
			GPUCommandBuffer = RHI->CreateGPUCommandBuffer(CommandSignature, (uint32)DrawArguments.size(), UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
			GPUCommandBuffer->SetResourceName("GPUCommandBuffer");

			uint32 CommandIndex = 0;
			for (const auto& Arg : DrawArguments)
			{
				GPUCommandBuffer->EncodeSetDrawIndexed(CommandIndex, 0,
					Arg.IndexCountPerInstance,
					Arg.InstanceCount,
					Arg.StartIndexLocation,
					Arg.BaseVertexLocation,
					Arg.StartInstanceLocation);
				++CommandIndex;
			}
			RHI->UpdateHardwareResourceGPUCommandBuffer(GPUCommandBuffer);

			// Occlude scene indirect draw command buffer
			GPUOccludeCommandBuffer = RHI->CreateGPUCommandBuffer(CommandSignature, (uint32)OccludeDrawArguments.size(), UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
			GPUOccludeCommandBuffer->SetResourceName("GPUOccludeCommandBuffer");

			CommandIndex = 0;
			for (const auto& Arg : OccludeDrawArguments)
			{
				GPUOccludeCommandBuffer->EncodeSetDrawIndexed(CommandIndex, 0,
					Arg.IndexCountPerInstance,
					Arg.InstanceCount,
					Arg.StartIndexLocation,
					Arg.BaseVertexLocation,
					Arg.StartInstanceLocation);
				++CommandIndex;
			}
			RHI->UpdateHardwareResourceGPUCommandBuffer(GPUOccludeCommandBuffer);
		}

		Inited = true;
	}
}