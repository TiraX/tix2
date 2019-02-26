/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FComputeTask : public IReferenceCounted
	{
	public:
		FComputeTask(const TString& ComputeShaderName);
		virtual ~FComputeTask();

	protected:

	protected:
		TString ShaderName;
	};
}
