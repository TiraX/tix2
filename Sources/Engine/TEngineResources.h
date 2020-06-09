	/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TEngineResources
	{
	public:
		static TTexturePtr EmptyTextureWhite;
		static TTexturePtr EmptyTextureBlack;
		static TTexturePtr EmptyTextureNormal;

		static void CreateGlobalResources();
		static void DestroyGlobalResources();
	};
}
