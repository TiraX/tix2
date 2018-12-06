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
	TResMaterialHelper::TResMaterialHelper()
		: VsFormat(EVSSEG_POSITION | EVSSEG_NORMAL | EVSSEG_TEXCOORD0 | EVSSEG_TANGENT)
		, BlendMode(BLEND_MODE_OPAQUE)
		, bDepthWrite(true)
		, bDepthTest(true)
		, bTwoSides(false)
		, DepthBuffer(EPF_UNKNOWN)
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
		Helper.SetBlendMode(GetBlendMode(BM.IsNull() ? "null" : BM.GetString()));

		// depth write / depth test / two sides
		Value& depth_write = Doc["depth_write"];
		Helper.EnableDepthWrite(depth_write.IsNull() ? true : depth_write.GetBool());

		Value& depth_test = Doc["depth_test"];
		Helper.EnableDepthTest(depth_test.IsNull() ? true : depth_test.GetBool());

		Value& two_sides = Doc["two_sides"];
		Helper.EnableTwoSides(two_sides.IsNull() ? false : two_sides.GetBool());

		// shader binding
		Value& shader_binding = Doc["shader_binding"];
		Helper.SetShaderBinding(shader_binding.IsNull() ? "" : shader_binding.GetString());

		// rt format
		Value& RT_Colors = Doc["rt_colors"];
		TI_ASSERT(RT_Colors.IsArray());
		Helper.ColorBuffers.resize(RT_Colors.Size());
		for (SizeType cb = 0; cb < RT_Colors.Size(); ++cb)
		{
			Helper.ColorBuffers[cb] = GetPixelFormat(RT_Colors[cb].GetString());
		}
		Value& RT_Depth = Doc["rt_depth"];
		Helper.DepthBuffer = GetPixelFormat(RT_Depth.GetString());

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

	void TResMaterialHelper::SetShaderBinding(const TString& SBRes)
	{
		ShaderBinding = SBRes;
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
			Define.ShaderBindingStr = AddStringToList(OutStrings, ShaderBinding);

			const int32 cb_count = (int32)ColorBuffers.size();
			int32 cb = 0;
			for (; cb < cb_count; ++cb)
			{
				Define.ColorBuffers[cb] = ColorBuffers[cb];
			}
			for (; cb < ERTC_COUNT; ++cb)
			{
				Define.ColorBuffers[cb] = EPF_UNKNOWN;
			}
			Define.DepthBuffer = DepthBuffer;

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
