/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	template<typename T>
	inline IInstrusivePtr<T> ResourceCast(FRenderResourcePtr Resource)
	{
		IInstrusivePtr<T> Ptr = static_cast<T*>(Resource.get());
		return Ptr;
	}

	class FRenderResource : public IReferenceCounted
	{
	public:
		FRenderResource()
		{}
		virtual ~FRenderResource() 
		{}

#if defined (TIX_DEBUG)
		void SetResourceName(const TString& Name)
		{
			ResourceName = Name;
		}
		const TString& GetResourceName() const
		{
			return ResourceName;
		}
#endif

	protected:
#if defined (TIX_DEBUG)
		TString ResourceName;
#endif
	};
}
