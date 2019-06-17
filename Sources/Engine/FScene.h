/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#include "FUniformBufferView.h"

namespace tix
{
	class FSceneLights;
	class FScene
	{
	public:
		FScene();
		~FScene();

		enum SceneFlag
		{
			ViewProjectionDirty = 1 << 0,
			ScenePrimitivesDirty = 1 << 1,
		};

		virtual void PrepareViewUniforms();

		void SetViewProjection(const FViewProjectionInfo& Info);
		void AddPrimitives(const TVector<FPrimitivePtr>& Primitives);
		void RemovePrimitives(const TVector<FPrimitivePtr>& Primitives);

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

		const TVector<FPrimitivePtr>& GetStaticDrawList(E_DRAWLIST_TYPE List) const
		{
			return StaticDrawLists[List];
		}

		FViewUniformBufferPtr GetViewUniformBuffer()
		{
			return ViewUniformBuffer;
		}
	protected:
		FSceneLights * SceneLights;

		TVector<FPrimitivePtr> StaticDrawLists[LIST_COUNT];

		// Scene flags per frame, will be cleared by the end of this frame
		uint32 SceneFlags;

		FViewProjectionInfo ViewProjection;

		FViewUniformBufferPtr ViewUniformBuffer;
	};
} // end namespace tix
