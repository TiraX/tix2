/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

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
		};

		void SetViewProjection(const FViewProjectionInfo& Info);
		void AddPrimitive(FPrimitivePtr Primitive);
		void RemovePrimitive(FPrimitivePtr Primitive);

		bool HasSceneFlag(SceneFlag Flag) const
		{
			return (SceneFlags & Flag) != 0;
		}

		void SetSceneFlag(SceneFlag Flag, bool Enable)
		{
			if (Enable)
			{
				SceneFlags |= Flag;
			}
			else
			{
				SceneFlags &= ~Flag;
			}
		}

		FSceneLights * GetSceneLights()
		{
			return SceneLights;
		}

		const FViewProjectionInfo& GetViewProjection() const
		{
			return ViewProjection;
		}

		const TVector<FPrimitivePtr>& GetStaticDrawList() const
		{
			return StaticDrawList;
		}
	protected:
		FSceneLights * SceneLights;

		TVector<FPrimitivePtr> StaticDrawList;

		uint32 SceneFlags;

		FViewProjectionInfo ViewProjection;
	};
} // end namespace tix
