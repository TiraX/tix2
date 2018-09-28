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
		FRenderResource()
		{}
		virtual ~FRenderResource() 
		{}

		virtual void Destroy() = 0;

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

	/////////////////////////////////////////////////////////////////////////////////

	class FRenderResourceInHeap : public FRenderResource
	{
	public:
		FRenderResourceInHeap(E_RENDER_RESOURCE_HEAP_TYPE InHeapType);
		virtual ~FRenderResourceInHeap();

		virtual void InitRenderResourceHeapSlot();

		E_RENDER_RESOURCE_HEAP_TYPE GetResourceHeapType() const
		{
			return HeapType;
		}

		uint32 GetRenderResourceSlot() const
		{
			return HeapSlot;
		}
	protected:
		E_RENDER_RESOURCE_HEAP_TYPE HeapType;
		uint32 HeapSlot;
	};
}
