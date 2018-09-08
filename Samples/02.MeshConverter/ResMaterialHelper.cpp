/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMaterialHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
#define ReturnEnumValue(Str, EnumValue) if (Str == #EnumValue) {return EnumValue;}
	E_BLEND_MODE GetMode(const TString& name)
	{
		ReturnEnumValue(name, BLEND_MODE_OPAQUE);
		ReturnEnumValue(name, BLEND_MODE_TRANSLUCENT);
		ReturnEnumValue(name, BLEND_MODE_MASK);
		ReturnEnumValue(name, BLEND_MODE_ADDITIVE);

		return BLEND_MODE_OPAQUE;
	}
	
	E_VERTEX_STREAM_SEGMENT GetVertexSegment(const TString& name)
	{
		ReturnEnumValue(name, EVSSEG_POSITION);
		ReturnEnumValue(name, EVSSEG_NORMAL);
		ReturnEnumValue(name, EVSSEG_COLOR);
		ReturnEnumValue(name, EVSSEG_TEXCOORD0);
		ReturnEnumValue(name, EVSSEG_TEXCOORD1);
		ReturnEnumValue(name, EVSSEG_TANGENT);
		ReturnEnumValue(name, EVSSEG_BLENDINDEX);
		ReturnEnumValue(name, EVSSEG_BLENDWEIGHT);

		return EVSSEG_POSITION;
	}

	TResMaterialHelper::TResMaterialHelper()
		: VsFormat(EVSSEG_POSITION | EVSSEG_NORMAL | EVSSEG_TEXCOORD0 | EVSSEG_TANGENT)
		, BlendMode(BLEND_MODE_OPAQUE)
		, bDepthWrite(true)
		, bDepthTest(true)
		, bTwoSides(false)
	{
	}

	TResMaterialHelper::~TResMaterialHelper()
	{
	}

	void TResMaterialHelper::LoadMaterial(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResMaterialHelper Helper;

		// shaders
		Value& Shaders = Doc["shaders"];
		TI_ASSERT(Shaders.IsArray() && Shaders.Size() == ESS_COUNT);
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			Helper.SetShaderName((E_SHADER_STAGE)s, Shaders[s].GetString());
		}

		// vs format
		Value& VSFormat = Doc["vs_format"];
		TI_ASSERT(VSFormat.IsArray());
		uint32 Format = 0;
		for (SizeType vs = 0; vs < VSFormat.Size(); ++vs)
		{
			Format |= GetVertexSegment(VSFormat[vs].GetString());
		}
		Helper.SetShaderVsFormat(Format);

		// blend mode
		Value& BM = Doc["blend_mode"];
		Helper.SetBlendMode(GetMode(BM.IsNull() ? "null" : BM.GetString()));

		// depth write / depth test / two sides
		Value& depth_write = Doc["depth_write"];
		Helper.EnableDepthWrite(depth_write.IsNull() ? true : depth_write.GetBool());

		Value& depth_test = Doc["depth_test"];
		Helper.EnableDepthTest(depth_test.IsNull() ? true : depth_test.GetBool());

		Value& two_sides = Doc["two_sides"];
		Helper.EnableTwoSides(two_sides.IsNull() ? false : two_sides.GetBool());

		Helper.OutputMaterial(OutStream, OutStrings);
	}


	void TResMaterialHelper::SetShaderName(E_SHADER_STAGE Stage, const TString& Name)
	{
		Shaders[Stage] = Name;
	}

	void TResMaterialHelper::SetBlendMode(E_BLEND_MODE InBlendMode)
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
				Define.ShaderCodeLength[s] = ShaderCodes[s].GetLength();
			}

			Define.VsFormat = VsFormat;
			Define.BlendMode = (uint8)BlendMode;
			Define.bDepthWrite = bDepthWrite ? 1 : 0;
			Define.bDepthTest = bDepthTest ? 1 : 0;
			Define.bTwoSides = bTwoSides ? 1 : 0;
			
			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderMaterial));

			// Write codes
			for (int32 s = 0; s < ESS_COUNT; ++s)
			{
				if (ShaderCodes[s].GetLength() > 0)
				{
					int32 write_len = ti_align4(ShaderCodes[s].GetLength());
					DataStream.Put(ShaderCodes[s].GetBuffer(), ShaderCodes[s].GetLength());
					FillZero4(DataStream);
				}
			}
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
