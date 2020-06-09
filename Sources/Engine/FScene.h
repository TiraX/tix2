/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "FUniformBufferView.h"

namespace tix
{
	struct FSceneMeshInfo
	{
		FSceneMeshInfo()
			: References(0)
		{
		}

		FSceneMeshInfo(int32 InRef, FMeshBufferPtr InOccMesh, FUniformBufferPtr InClusterData)
			: References(InRef)
			, OccludeMesh(InOccMesh)
			, ClusterData(InClusterData)
		{
		}

		~FSceneMeshInfo()
		{
			OccludeMesh = nullptr;
			ClusterData = nullptr;
		}

		int32 References;
		FMeshBufferPtr OccludeMesh;
		FUniformBufferPtr ClusterData;
	};

	class FSceneLights;
	class FScene
	{
	public:
		FScene();
		~FScene();

		// Scene flags clear every frames
		enum SceneFlag
		{
			ViewProjectionDirty = 1 << 0,
			ScenePrimitivesDirty = 1 << 1,
			EnvironmentDirty = 1 << 2,
			SceneTileDirty = 1 << 3,

			ViewUniformDirty = (ViewProjectionDirty | EnvironmentDirty),
		};

		TI_API void InitRenderFrame();

		TI_API void SetViewProjection(const FViewProjectionInfo& Info);
		TI_API void SetEnvironmentInfo(const FEnvironmentInfo& Info);

		// Add / Remove primitives from the same static mesh
		void AddStaticMeshPrimitives(const TVector<FPrimitivePtr>& Primitives);
		void RemoveStaticMeshPrimitives(const TVector<FPrimitivePtr>& Primitives);

		void AddSceneTileInfo(FSceneTileResourcePtr SceneTileResource);
		void RemoveSceneTileInfo(FSceneTileResourcePtr SceneTileResource);

		void AddSceneMeshBuffer(FMeshBufferPtr InMesh, FMeshBufferPtr InOccludeMesh, FUniformBufferPtr InClusterData);
		void RemoveSceneMeshBuffer(FMeshBufferPtr InMesh);

		bool HasSceneFlag(SceneFlag Flag) const
		{
			return (SceneFlags & Flag) != 0;
		}

		void SetSceneFlag(SceneFlag Flag)
		{
			SceneFlags |= Flag;
		}

		void ClearSceneFlags()
		{
			SceneFlags = 0;
		}

		FSceneLights * GetSceneLights()
		{
			return SceneLights;
		}

		const FViewProjectionInfo& GetViewProjection() const
		{
			return ViewProjection;
		}

		const FEnvironmentInfo& GetEnvironmentInfo() const
		{
			return EnvInfo;
		}

		const TVector<FPrimitivePtr>& GetStaticDrawList(E_DRAWLIST_TYPE List) const
		{
			TI_ASSERT(0);
			return StaticDrawLists[List];
		}

		const THMap<vector2di, FSceneTileResourcePtr>& GetSceneTiles() const
		{
			return SceneTiles;
		}

		FViewUniformBufferPtr GetViewUniformBuffer()
		{
			return ViewUniformBuffer;
		}

		const THMap<FMeshBufferPtr, FSceneMeshInfo>& GetSceneMeshes() const
		{
			return SceneMeshes;
		}
	private:
		void PrepareViewUniforms();

	private:
		FSceneLights * SceneLights;

		// Scene flags per frame, will be cleared by the end of this frame
		uint32 SceneFlags;

		FViewProjectionInfo ViewProjection;
		FEnvironmentInfo EnvInfo;

		// Uniform buffers
		FViewUniformBufferPtr ViewUniformBuffer;

		// Scene meshes
		THMap<FMeshBufferPtr, FSceneMeshInfo> SceneMeshes;

		// Scene tiles
		THMap<vector2di, FSceneTileResourcePtr> SceneTiles;

		TVector<FPrimitivePtr> StaticDrawLists[LIST_COUNT];
	};
} // end namespace tix
