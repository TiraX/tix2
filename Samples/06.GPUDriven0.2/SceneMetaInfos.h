/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FSceneMetaInfos
	{
	public:
		FSceneMetaInfos();
		~FSceneMetaInfos();

		void PrepareSceneResources(FRHI* RHI, FScene * Scene, FGPUCommandSignaturePtr CommandSignature);

		FInstanceBufferPtr GetMergedInstanceBuffer()
		{
			return MergedInstanceBuffer;
		}
		FMeshBufferPtr GetMergedSceneMeshBuffer()
		{
			return MergedMeshBuffer;
		}
		FGPUCommandBufferPtr GetGPUCommandBuffer()
		{
			return GPUCommandBuffer;
		}
	private:


	private:
		FInstanceBufferPtr MergedInstanceBuffer;
		FMeshBufferPtr MergedMeshBuffer;
		FGPUCommandBufferPtr GPUCommandBuffer;

		friend class FScene;
	};
} // end namespace tix
