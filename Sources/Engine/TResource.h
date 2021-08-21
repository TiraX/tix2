/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RESOURCE_TYPE
	{
		ERES_MESH_BUFFER,
		ERES_TEXTURE,
		ERES_PIPELINE,
        ERES_TILEPIPELINE,
		ERES_MATERIAL_INSTANCE,
		ERES_RENDER_TARGET,
		ERES_SHADER_BINDING,
		ERES_SHADER,
		ERES_INSTANCE,
		ERES_SCENE_TILE,
		ERES_COLLISION,
		ERES_STATIC_MESH,
		ERES_SKELETON,
		ERES_ANIM_SEQUENCE,
		ERES_RTX_PIPELINE,

		ERES_COUNT,
	};
	class TI_API TResource : public IReferenceCounted
	{
	public:
		TResource(E_RESOURCE_TYPE Type)
			: ResType(Type)
		{}
		virtual ~TResource()
		{}

		virtual void InitRenderThreadResource() = 0;
		virtual void DestroyRenderThreadResource() = 0;

		E_RESOURCE_TYPE GetType() const
		{
			return ResType;
		}

		const TString& GetResourceName() const
		{
#ifdef TIX_DEBUG
			return ResourceName;
#else
			static const TString s_Resource = "TixResource";
			return s_Resource;
#endif
		}

		void SetResourceName(const TString& ResName)
		{
#ifdef TIX_DEBUG
			ResourceName = ResName;
#else
#endif
		}

	protected:
		E_RESOURCE_TYPE ResType;
#ifdef TIX_DEBUG
		TString ResourceName;
#endif
		friend class TAssetFile;
	};
}
