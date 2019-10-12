/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RENDER_RESOURCE_TYPE
	{
		RRT_VERTEX_BUFFER,
		RRT_INSTANCE_BUFFER,
		RRT_UNIFORM_BUFFER,
		RRT_TEXTURE,
		RRT_PIPELINE,
		RRT_SHADER,
		RRT_SHADER_BINDING,
		RRT_RENDER_TARGET,
		RRT_RESOURCE_TABLE,
		RRT_ARGUMENT_BUFFER,
		RRT_GPU_COMMAND_SIGNATURE,
		RRT_GPU_COMMAND_BUFFER,
		RRT_SCENE_TILE,
	};

	template<typename T>
	inline IInstrusivePtr<T> ResourceCast(FRenderResourcePtr Resource)
	{
		IInstrusivePtr<T> Ptr = static_cast<T*>(Resource.get());
		return Ptr;
	}

	class FRenderResource : public IReferenceCounted
	{
	public:
		FRenderResource(E_RENDER_RESOURCE_TYPE InResourceType)
			: ResourceType(InResourceType)
		{}
		virtual ~FRenderResource() 
		{}

		E_RENDER_RESOURCE_TYPE GetResourceType() const
		{
			return ResourceType;
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
		E_RENDER_RESOURCE_TYPE ResourceType;

#if defined (TIX_DEBUG)
		TString ResourceName;
#endif
	};
}
