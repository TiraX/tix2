/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// FEnvLight: Hold indirect light info
	// Filtered cubemap for indirect specular, and qiuxie for indirect diffuse
	class FEnvLight : public FRenderResource
	{
	public:
		FEnvLight(FTexturePtr InCubemap, const vector3df& InPosition);
		virtual ~FEnvLight();

	private:
		FTexturePtr EnvCubemap;
		vector3df Position;
	};
}
