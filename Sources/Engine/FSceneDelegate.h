/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FSceneDelegate
	{
	public:
		FSceneDelegate() {};
		virtual ~FSceneDelegate() {};

		virtual void OnAddPrimitive(FPrimitivePtr InPrimitive) {};
		virtual void OnRemovePrimitive(FPrimitivePtr InPrimitive) {};

		virtual void OnAddSceneTile(TSceneTileResourcePtr InSceneTileRes) {};
		virtual void OnRemoveSceneTile(TSceneTileResourcePtr InSceneTileRes) {};

	};
} // end namespace tix
