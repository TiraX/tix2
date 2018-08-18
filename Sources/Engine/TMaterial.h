/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	//class FMaterial;
	//typedef TI_INTRUSIVE_PTR(FMaterial) FMaterialPtr;

	class TMaterial : public TResource
	{
	public:
		TMaterial();
		virtual ~TMaterial();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		//TMaterialDesc Desc;
		//FMaterialPtr TextureResource;

	protected:

	protected:
	};

	typedef TI_INTRUSIVE_PTR(TMaterial) TMaterialPtr;

}
