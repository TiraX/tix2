/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FRenderResource : public IReferenceCounted
	{
	public:
		FRenderResource(E_RESOURCE_FAMILY InFamily)
			: ResourceFamily(InFamily)
		{}
		virtual ~FRenderResource() 
		{
			TI_ASSERT(IsRenderThread());
		}

		E_RESOURCE_FAMILY GetResourceFamily() const
		{
			return ResourceFamily;
		}
#if defined (TIX_DEBUG)
		void SetResourceName(const int8* Name)
		{
			ResourceName = Name;
			ResourceNameW = FromString(ResourceName);
		}
		const TString& GetResourceName() const
		{
			return ResourceName;
		}
		const TWString& GetResourceWName() const
		{
			return ResourceNameW;
		}
#endif
	protected:

	protected:
		E_RESOURCE_FAMILY	ResourceFamily;
#if defined (TIX_DEBUG)
		TString				ResourceName;
		TWString			ResourceNameW;
#endif
	};
}
