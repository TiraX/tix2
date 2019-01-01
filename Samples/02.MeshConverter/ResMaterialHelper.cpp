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

		// stencil state
		Value& stencil_enable = Doc["stencil_enable"];
		if (!stencil_enable.IsNull())
		{
			if (stencil_enable.GetBool())
				Helper.PipelineDesc.Enable(EPSO_STENCIL);
			else
				Helper.PipelineDesc.Disable(EPSO_STENCIL);
		}

		Value& stencil_read_mask = Doc["stencil_read_mask"];
		if (!stencil_read_mask.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.StencilReadMask = (uint8)stencil_read_mask.GetInt();
		}

		Value& stencil_write_mask = Doc["stencil_write_mask"];
		if (!stencil_write_mask.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.StencilWriteMask = (uint8)stencil_write_mask.GetInt();
		}

		Value& front_stencil_fail = Doc["front_stencil_fail"];
		if (!front_stencil_fail.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.FrontFace.StencilFailOp = GetStencilOp(front_stencil_fail.GetString());
		}

		Value& front_stencil_depth_fail = Doc["front_stencil_depth_fail"];
		if (!front_stencil_depth_fail.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.FrontFace.StencilDepthFailOp = GetStencilOp(front_stencil_depth_fail.GetString());
		}

		Value& front_stencil_pass = Doc["front_stencil_pass"];
		if (!front_stencil_pass.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.FrontFace.StencilPassOp = GetStencilOp(front_stencil_pass.GetString());
		}

		Value& front_stencil_func = Doc["front_stencil_func"];
		if (!front_stencil_func.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.FrontFace.StencilFunc = GetComparisonFunc(front_stencil_func.GetString());
		}

		Value& back_stencil_fail = Doc["back_stencil_fail"];
		if (!back_stencil_fail.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.BackFace.StencilFailOp = GetStencilOp(back_stencil_fail.GetString());
		}

		Value& back_stencil_depth_fail = Doc["back_stencil_depth_fail"];
		if (!back_stencil_depth_fail.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.BackFace.StencilDepthFailOp = GetStencilOp(back_stencil_depth_fail.GetString());
		}

		Value& back_stencil_pass = Doc["back_stencil_pass"];
		if (!back_stencil_pass.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.BackFace.StencilPassOp = GetStencilOp(back_stencil_pass.GetString());
		}

		Value& back_stencil_func = Doc["back_stencil_func"];
		if (!back_stencil_func.IsNull())
		{
			Helper.PipelineDesc.DepthStencilDesc.BackFace.StencilFunc = GetComparisonFunc(back_stencil_func.GetString());
		}

		// shader binding
		Value& shader_binding = Doc["shader_binding"];
		Helper.SetShaderBinding(shader_binding.IsNull() ? "" : shader_binding.GetString());

		// rt format
		Value& RT_Colors = Doc["rt_colors"];
		TI_ASSERT(RT_Colors.IsArray() && RT_Colors.Size() <= 4);

		Helper.PipelineDesc.RTCount = (int32)RT_Colors.Size();
		for (SizeType cb = 0; cb < RT_Colors.Size(); ++cb)
		{
			Helper.PipelineDesc.RTFormats[cb] = GetPixelFormat(RT_Colors[cb].GetString());
		}
		Value& RT_Depth = Doc["rt_depth"];
		Helper.PipelineDesc.DepthFormat = GetPixelFormat(RT_Depth.GetString());

		Helper.OutputMaterial(OutStream, OutStrings);
	}


	void TResMaterialHelper::SetShaderName(E_SHADER_STAGE Stage, const TString& Name)
	{
		PipelineDesc.ShaderName[Stage] = Name;
	}

	void TResMaterialHelper::SetBlendMode(E_BLEND_MODE InBlendMode)
	{
		switch (InBlendMode)
		{
		case BLEND_MODE_OPAQUE:
		case BLEND_MODE_MASK:
			PipelineDesc.Disable(EPSO_BLEND);
			break;
		case BLEND_MODE_TRANSLUCENT:
			PipelineDesc.Enable(EPSO_BLEND);
			PipelineDesc.BlendState.SrcBlend = EBF_SRC_ALPHA;
			PipelineDesc.BlendState.DestBlend = EBF_ONE_MINUS_SRC_ALPHA;
			PipelineDesc.BlendState.BlendOp = EBE_FUNC_ADD;
			PipelineDesc.BlendState.SrcBlendAlpha = EBF_ONE;
			PipelineDesc.BlendState.DestBlendAlpha = EBF_ZERO;
			PipelineDesc.BlendState.BlendOpAlpha = EBE_FUNC_ADD;
			break;
		case BLEND_MODE_ADDITIVE:
			PipelineDesc.Enable(EPSO_BLEND);
			PipelineDesc.BlendState.SrcBlend = EBF_SRC_ALPHA;
			PipelineDesc.BlendState.DestBlend = EBF_ONE;
			PipelineDesc.BlendState.BlendOp = EBE_FUNC_ADD;
			PipelineDesc.BlendState.SrcBlendAlpha = EBF_ONE;
			PipelineDesc.BlendState.DestBlendAlpha = EBF_ZERO;
			PipelineDesc.BlendState.BlendOpAlpha = EBE_FUNC_ADD;
			break;
        default:
            TI_ASSERT(0);
            break;
		}
	}

	void TResMaterialHelper::SetShaderVsFormat(uint32 InVsFormat)
	{
		PipelineDesc.VsFormat = InVsFormat;
	}

	void TResMaterialHelper::EnableDepthWrite(bool bEnable)
	{
		if (bEnable)
			PipelineDesc.Enable(EPSO_DEPTH);
		else
			PipelineDesc.Disable(EPSO_DEPTH);
	}

	void TResMaterialHelper::EnableDepthTest(bool bEnable)
	{
		if (bEnable)
			PipelineDesc.Enable(EPSO_DEPTH_TEST);
		else
			PipelineDesc.Disable(EPSO_DEPTH_TEST);
	}

	void TResMaterialHelper::EnableTwoSides(bool bEnable)
	{
		if (bEnable)
			PipelineDesc.RasterizerDesc.CullMode = ECM_NONE;
		else
			PipelineDesc.RasterizerDesc.CullMode = ECM_BACK;
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
				Define.ShaderNames[s] = AddStringToList(OutStrings, PipelineDesc.ShaderName[s]);
				Define.ShaderCodeLength[s] = ShaderCodes[s].GetLength();
			}

			Define.Flags = PipelineDesc.Flags;
			Define.BlendState = PipelineDesc.BlendState;
			Define.RasterizerDesc = PipelineDesc.RasterizerDesc;
			Define.DepthStencilDesc = PipelineDesc.DepthStencilDesc;
			Define.VsFormat = PipelineDesc.VsFormat;

			int32 cb = 0;
			for (; cb < ERTC_COUNT; ++cb)
			{
				Define.ColorBuffers[cb] = PipelineDesc.RTFormats[cb];
			}
			Define.DepthBuffer = PipelineDesc.DepthFormat;

			Define.ShaderBindingStr = AddStringToList(OutStrings, ShaderBinding);

			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderMaterial));

			// Write codes
			for (int32 s = 0; s < ESS_COUNT; ++s)
			{
				if (ShaderCodes[s].GetLength() > 0)
				{
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
