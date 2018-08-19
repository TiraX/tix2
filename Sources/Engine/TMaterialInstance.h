/*
TiX Engine v2.0 Copyright (C) 2018
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_MI_PARAM_TYPE
	{
		MIPT_UNKNOWN,
		MIPT_INT,
		MIPT_FLOAT,
		MIPT_INT4,
		MIPT_FLOAT4,
		MIPT_TEXTURE,

		MIPT_COUNT,
	};
	//class FMaterial;
	//typedef TI_INTRUSIVE_PTR(FMaterial) FMaterialPtr;

	class TMaterialInstance : public TResource
	{
	public:
		TMaterialInstance();
		virtual ~TMaterialInstance();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		int32 GetValueBufferLength() const;
		//TMaterialDesc Desc;
		//FMaterialPtr TextureResource;

	protected:
		TVector<TString> ParamNames;
		TVector<int32> ParamTypes;
		TStream ParamValueBuffer;

	protected:
		friend class TResFile;
	};
}
