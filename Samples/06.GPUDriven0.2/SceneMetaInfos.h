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


	private:


	private:
		uint32 SceneMetaFlags;

		friend class FScene;
	};
} // end namespace tix
