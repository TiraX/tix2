/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResource : public IReferenceCounted
	{
	public:
		TResource()
		{}
		virtual ~TResource()
		{}

#ifdef TI_DEBUG
		TString ResourceName;
#endif
	};
	typedef TI_INTRUSIVE_PTR(TResource) TResourcePtr;

}