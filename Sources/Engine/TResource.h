/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RESOURCE_TYPE
	{
		ERES_MESH,
		ERES_TEXTURE,

		ERES_COUNT,
	};
	class TResource : public IReferenceCounted
	{
	public:
		TResource(E_RESOURCE_TYPE Type)
			: ResType(Type)
		{}
		virtual ~TResource()
		{}

		virtual void InitRenderThreadResource() = 0;
		virtual void DestroyRenderThreadResource() = 0;

	protected:
		E_RESOURCE_TYPE ResType;
#ifdef TI_DEBUG
		TString ResourceName;
#endif
		friend class TResFile;
	};
	typedef TI_INTRUSIVE_PTR(TResource) TResourcePtr;

}