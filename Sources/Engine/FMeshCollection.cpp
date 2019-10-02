/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FMeshCollection.h"

namespace tix
{
	FMeshCollection::FMeshCollection()
		: VertexCollected(0)
		, IndexCollected(0)
		, MeshCollected(0)
	{
		Init();
	}

	FMeshCollection::~FMeshCollection()
	{
	}

	void FMeshCollection::Init()
	{
		// Create a mesh buffer
		FRHI * RHI = FRHI::Get();
		MeshBuffer = RHI->CreateEmptyMeshBuffer(EPT_TRIANGLELIST, VertexFormat, VertexDataCount, IndexFormat, IndexDataCount);

		const uint32 VertexStride = TMeshBuffer::GetStrideFromFormat(VertexFormat);
		const uint32 VertexDataSize = VertexStride * VertexDataCount;

		RHI->UpdateHardwareResourceMesh(
			MeshBuffer,
			VertexDataSize,
			VertexStride,
			IndexDataCount * sizeof(uint32),
			IndexFormat,
			TString("StaticMeshCollection"));

		// Create meta data uniform buffer
		MetaData = RHI->CreateUniformBuffer(sizeof(FMeshMetaData), MaxMeshes);
		RHI->UpdateHardwareResourceUB(MetaData, nullptr);

		// Create GPU command buffer
		//GPUCommandBuffer = RHI->CreateGPUCommandBuffer();

	}

	void FMeshCollection::AddPrimitives(TVector<FPrimitivePtr>& InPrims)
	{
		return;
		TI_TODO("Make a big big refactory here. Collect meshes from loading, NOT adding to scene.");
		// Add vertex and index data to mesh buffer
		FRHI * RHI = FRHI::Get();
		const uint32 VertexStride = TMeshBuffer::GetStrideFromFormat(VertexFormat);

		TVector<FMeshMetaData> MeshMetaData;
		MeshMetaData.resize(InPrims.size());
		for (uint32 i = 0 ; i < (uint32)InPrims.size() ; ++ i)
		{
			FPrimitivePtr Prim = InPrims[i];
			FMeshBufferPtr PrimMB = Prim->GetMeshBuffer();
			TI_ASSERT(PrimMB->GetVSFormat() == VertexFormat);
			TI_ASSERT(PrimMB->GetIndexType() == IndexFormat);

			FMeshBuffer* PrimMBPtr = PrimMB.get();
			if (MeshBufferCollected.find(PrimMBPtr) == MeshBufferCollected.end())
			{
				// Move vertex data to MeshBuffer
				const int32 VertexCount = PrimMB->GetVerticesCount();

				RHI->CopyBufferRegion(MeshBuffer,
					VertexCollected * VertexStride,
					IndexCollected * IndexStride,
					PrimMB,
					0, PrimMB->GetVerticesCount() * VertexStride,
					0, PrimMB->GetIndicesCount() * IndexStride);

				// Collect meta data
				FMeshMetaData Meta;
				Meta.VertexOffset = VertexCollected * VertexStride;
				Meta.IndexOffset = IndexCollected * IndexStride;
				Meta.IndexCount = PrimMB->GetIndicesCount();
				Meta.BBox = Prim->GetBBox();
				MeshMetaData[i] = Meta;

				// Remember how many vertices are collected
				VertexCollected += PrimMB->GetVerticesCount();
				TI_ASSERT(VertexCollected <= VertexDataCount);
				IndexCollected += PrimMB->GetIndicesCount();
				TI_ASSERT(IndexCollected <= IndexDataCount);

				MeshBufferCollected[PrimMBPtr] = Meta;
			}
			else
			{
				MeshMetaData[i] = MeshBufferCollected[PrimMBPtr];
			}
		}

		// Create meta data resource.
		FUniformBufferPtr Upload = RHI->CreateUniformBuffer(sizeof(FMeshMetaData), (uint32)MeshMetaData.size());
		RHI->UpdateHardwareResourceUB(Upload, MeshMetaData.data());
		RHI->CopyBufferRegion(MetaData, MeshCollected * sizeof(FMeshMetaData), Upload, (uint32)MeshMetaData.size() * sizeof(FMeshMetaData));
		MeshCollected += (uint32)MeshMetaData.size();
		//TI_ASSERT(0);
	}
}
