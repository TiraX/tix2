/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FPipeline : public FRenderResource
	{
	public:
		FPipeline(E_RESOURCE_FAMILY InFamily);
		virtual ~FPipeline();

		virtual void Destroy() override {};

	protected:

	protected:
	};
}
