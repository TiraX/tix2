/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RESOURCE_TYPE
	{
		ERES_MESH,
		ERES_TEXTURE,
		ERES_PIPELINE,
		ERES_MATERIAL_INSTANCE,
		ERES_RENDER_TARGET,
		ERES_SHADER_BINDING,
		ERES_SHADER,

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
		friend class TResourceFile;
	};

	class TResourceTask : public IReferenceCounted
	{
	public:
		TResourceTask()
		{}

		~TResourceTask()
		{
			SourceFile = nullptr;
			for (auto& Res : Resources)
			{
				Res = nullptr;
			}
		}

		TResource* GetResourcePtr(int32 Index = 0)
		{
			return Resources[Index].get();
		}

		void InitRenderThreadResource()
		{
			for (auto& Res : Resources)
			{
				Res->InitRenderThreadResource();
			}
		}

		void DestroyRenderThreadResource()
		{
			for (auto& Res : Resources)
			{
				Res->DestroyRenderThreadResource();
			}
		}

		void ClearResources()
		{
			for (auto& Res : Resources)
			{
				Res = nullptr;
			}
			Resources.clear();
		}

		bool HasReference() const
		{
			for (auto& Res : Resources)
			{
				if (Res->referenceCount() > 1)
				{
					return true;
				}
			}
			return false;
		}
		
		// Hold an source file for asynchronous loading
		TResourceFilePtr SourceFile;
		// Resource loaded
		TVector<TResourcePtr> Resources;
	};
}