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

		void AddSceneTileInfo(FSceneTileResourcePtr SceneTileResource);
		void RemoveSceneTileInfo(FSceneTileResourcePtr SceneTileResource);

		void AddSceneMeshBuffer(FMeshBufferPtr InMesh, FMeshBufferPtr InOccludeMesh, FUniformBufferPtr InClusterData);
		void RemoveSceneMeshBuffer(FMeshBufferPtr InMesh);

		void AddEnvLight(FTexturePtr CubeTexture, const vector3df& Position);
		void RemoveEnvLight(FEnvLightPtr InEnvLight);

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

		FSceneLights* GetSceneLights()
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

		FEnvLightPtr FindNearestEnvLight(const vector3df& Pos)
		{
			return EnvLight;
		}
	private:
		void PrepareViewUniforms();
		void UpdateAccelerationStructure();

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

		// Scene TLAS
		FTopLevelAccelerationStructurePtr SceneTLAS;

		// Env Lights, leave ONE envlight temp, should support multi env light in futher
		FEnvLightPtr EnvLight;
	};
} // end namespace tix
