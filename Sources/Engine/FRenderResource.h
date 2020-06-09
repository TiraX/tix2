/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
		enum E_USAGE
		{
			// By default, 
			// vertex data is D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, 
			// index data is D3D12_RESOURCE_STATE_INDEX_BUFFER
			// instance data is D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
			USAGE_DEFAULT,

			// vertex and index data used as copy source
			USAGE_COPY_SOURCE
		};

		FRenderResource(E_RENDER_RESOURCE_TYPE InResourceType)
			: ResourceType(InResourceType)
			, Usage(USAGE_DEFAULT)
		{}
		virtual ~FRenderResource() 
		{}

		E_RENDER_RESOURCE_TYPE GetResourceType() const
		{
			return ResourceType;
		}

		E_USAGE GetUsage() const
		{
			return Usage;
		}

		void SetUsage(E_USAGE InUsage)
		{
			Usage = InUsage;
		}

		virtual void SetResourceName(const TString& Name)
		{
#if defined (TIX_DEBUG)
			ResourceName = Name;
#endif
		}
		const TString& GetResourceName() const
		{
#if defined (TIX_DEBUG)
			return ResourceName;
#else
			static const TString DefaultResourceName = "NoNamedTiXResource";
			return DefaultResourceName;
#endif
		}

	protected:
		E_RENDER_RESOURCE_TYPE ResourceType;
		E_USAGE Usage;

#if defined (TIX_DEBUG)
		TString ResourceName;
#endif
	};
}
