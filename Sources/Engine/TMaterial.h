/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TMaterial : public TResource
	{
	public:
		TMaterial();
		virtual ~TMaterial();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		TPipelinePtr Pipeline;

	protected:

	protected:
	};
}
