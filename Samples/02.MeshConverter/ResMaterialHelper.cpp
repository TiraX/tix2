/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMaterialHelper.h"

namespace tix
{
	TResMaterialHelper::TResMaterialHelper()
		: VsFormat(EVSSEG_POSITION | EVSSEG_NORMAL | EVSSEG_TEXCOORD0 | EVSSEG_TANGENT)
		, BlendMode(TMaterial::MATERIAL_BLEND_OPAQUE)
		, bDepthWrite(true)
		, bDepthTest(true)
		, bTwoSides(false)
	{
	}

	TResMaterialHelper::~TResMaterialHelper()
	{
	}


	void TResMaterialHelper::SetShaderName(E_SHADER_STAGE Stage, const TString& Name)
	{
		Shaders[Stage] = Name;
	}

	void TResMaterialHelper::SetBlendMode(TMaterial::E_BLEND_MODE InBlendMode)
	{
		BlendMode = InBlendMode;
	}

	void TResMaterialHelper::SetShaderVsFormat(uint32 InVsFormat)
	{
		VsFormat = InVsFormat;
	}

	void TResMaterialHelper::EnableDepthWrite(bool bEnable)
	{
		bDepthWrite = bEnable;
	}

	void TResMaterialHelper::EnableDepthTest(bool bEnable)
	{
		bDepthTest = bEnable;
	}

	void TResMaterialHelper::EnableTwoSides(bool bEnable)
	{
		bTwoSides = bEnable;
	}
	
	void TResMaterialHelper::OutputMaterial(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_MATERIAL;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_MATERIAL;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderMaterial Define;
			for (int32 s = 0; s < ESS_COUNT; ++s)
			{
				Define.ShaderNames[s] = AddStringToList(OutStrings, Shaders[s]);
			}

			Define.VsFormat = VsFormat;
			Define.BlendMode = (uint8)BlendMode;
			Define.bDepthWrite = bDepthWrite ? 1 : 0;
			Define.bDepthTest = bDepthTest ? 1 : 0;
			Define.bTwoSides = bTwoSides ? 1 : 0;
			
			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderMaterial));
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
