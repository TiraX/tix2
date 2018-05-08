/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FShader : public FRenderResource
	{
	public:
		FShader(E_RESOURCE_FAMILY InFamily);
		virtual ~FShader();

	protected:

	protected:
	};

	typedef TI_INTRUSIVE_PTR(FShader) FShaderPtr;
}
