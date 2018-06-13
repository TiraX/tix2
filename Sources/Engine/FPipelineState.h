/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FPipelineState : public IReferenceCounted
	{
	public:
		FPipelineState();
		virtual ~FPipelineState();

	protected:

	protected:
	};

	typedef TI_INTRUSIVE_PTR(FPipelineState) FPipelineStatePtr;
}
