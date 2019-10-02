/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FMeshCollection
	{
	public:
		FMeshCollection();
		~FMeshCollection();

		static const uint32 VertexFormat = EVSSEG_POSITION | EVSSEG_NORMAL | EVSSEG_TEXCOORD0 | EVSSEG_TANGENT;
		static const uint32 VertexDataCount = 512 * 1024;
		static const uint32 IndexDataCount = 2 * 1024 * 1024;
		static const E_INDEX_TYPE IndexFormat = EIT_16BIT;
		static const uint32 IndexStride = 2;
		static const uint32 MaxMeshes = 2 * 1024;

		void AddPrimitives(TVector<FPrimitivePtr>& InPrims);

	private:
		void Init();

	private:
		struct FMeshMetaData
		{
			uint32 VertexOffset;
			uint32 IndexOffset;
			uint32 IndexCount;
			aabbox3df BBox;
		};
		FMeshBufferPtr MeshBuffer;
		FUniformBufferPtr MetaData;
		FGPUCommandBufferPtr GPUCommandBuffer;

		THMap<FMeshBuffer*, FMeshMetaData> MeshBufferCollected;

		uint32 VertexCollected;
		uint32 IndexCollected;
		uint32 MeshCollected;
	};
} // end namespace tix
