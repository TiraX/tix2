/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SceneMetaInfos.h"

namespace tix
{
	FSceneMetaInfos::FSceneMetaInfos()
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
			uint32 TotalVertexCount = 0;
			uint32 TotalIndexCount = 0;
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
						TotalVertexCount += MeshBuffer->GetVerticesCount();
						TotalIndexCount += MeshBuffer->GetIndicesCount();
						TI_ASSERT(MeshBuffer->GetIndexType() == EIT_32BIT);	// Compute shader can only access 32bit aligned data
						TI_ASSERT(VBFormat == 0 || VBFormat == MeshBuffer->GetVSFormat());
						VBFormat = MeshBuffer->GetVSFormat();
						++TotalLoadedMeshes;
					}
				}
				TI_ASSERT(TotalVertexCount > 0 && TotalIndexCount > 0);
			}

			TI_ASSERT(TotalInstances > 0 && TotalLoadedMeshes > 0);

			// Allocate space for draw arguments
			TVector<FDrawInstanceArgument> DrawArguments;
			DrawArguments.resize(TotalLoadedMeshes);

			// Merge meshes and instances
			{
				// Create MergedMeshBuffer resource
				const uint32 VertexStride = TMeshBuffer::GetStrideFromFormat(VBFormat);
				MergedMeshBuffer = RHI->CreateEmptyMeshBuffer(EPT_TRIANGLELIST, VBFormat, TotalVertexCount, EIT_32BIT, TotalIndexCount);
				MergedMeshBuffer->SetResourceName("MergedMeshBuffer");
				RHI->UpdateHardwareResourceMesh(MergedMeshBuffer, TotalVertexCount * VertexStride, VertexStride, TotalIndexCount * sizeof(uint32), EIT_32BIT, "MergedMeshBuffer");

				// Create MergedInstanceBuffer resource
				MergedInstanceBuffer = RHI->CreateEmptyInstanceBuffer(TotalInstances, TInstanceBuffer::InstanceStride);
				MergedInstanceBuffer->SetResourceName("MergedInstanceBuffer");
				RHI->UpdateHardwareResourceIB(MergedInstanceBuffer, nullptr);
			
				uint32 VBOffset = 0, IBOffset = 0;
				uint32 PrimIndex = 0;
				uint32 InstanceDstOffset = 0;
				for (const auto& T : SceneTileResources)
				{
					FSceneTileResourcePtr TileRes = T.second;

					// Merge meshes
					{
						const TVector<FPrimitivePtr>& TilePrimitives = TileRes->GetPrimitives();
						TI_TODO("Collected mesh buffer may be duplicated. Remove duplicated mesh. Copy a unique ONE.");

						// Copy mesh vertex and index to MergedMeshBuffer 
						// And record draw arguments
						for (auto Prim : TileRes->GetPrimitives())
						{
							if (Prim != nullptr)
							{
								FMeshBufferPtr MeshBuffer = Prim->GetMeshBuffer();
								RHI->SetResourceStateMB(MeshBuffer, RESOURCE_STATE_COPY_SOURCE);

								RHI->CopyBufferRegion(
									MergedMeshBuffer,
									VBOffset,
									IBOffset,
									MeshBuffer,
									0,
									MeshBuffer->GetVerticesCount() * VertexStride,
									0,
									MeshBuffer->GetIndicesCount() * sizeof(uint32));

								// Remember draw arguments
								FDrawInstanceArgument& DrawArg = DrawArguments[PrimIndex];
								DrawArg.IndexCountPerInstance = MeshBuffer->GetIndicesCount();
								DrawArg.InstanceCount = Prim->GetInstanceCount();
								DrawArg.StartIndexLocation = IBOffset / sizeof(uint32);
								DrawArg.BaseVertexLocation = VBOffset / VertexStride;
								DrawArg.StartInstanceLocation = InstanceDstOffset + Prim->GetInstanceOffset();

								// Remember offsets
								VBOffset += MeshBuffer->GetVerticesCount() * VertexStride;
								IBOffset += MeshBuffer->GetIndicesCount() * sizeof(uint32);

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
			}

			// Create scene indirect draw command buffer
			{
				GPUCommandBuffer = RHI->CreateGPUCommandBuffer(CommandSignature, DrawArguments.size(), UB_FLAG_GPU_COMMAND_BUFFER_RESOURCE);
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
			}
		}
	}
}
