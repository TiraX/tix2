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
		}

		virtual void Destroy() = 0;

		E_RESOURCE_FAMILY GetResourceFamily() const
		{
			return ResourceFamily;
		}
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

	protected:
		E_RESOURCE_FAMILY	ResourceFamily;
#if defined (TIX_DEBUG)
		TString				ResourceName;
#endif
	};
}
