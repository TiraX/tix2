/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	TMaterial::TMaterial()
	{}

	TMaterial::~TMaterial()
	{
	}

	void TMaterial::SetBlendMode(E_BLEND_MODE InBlendMode)
	{
		switch (InBlendMode)
		{
		case BLEND_MODE_OPAQUE:
			Desc.Disable(EPSO_BLEND);
			break;
		case BLEND_MODE_TRANSLUCENT:
			Desc.Enable(EPSO_BLEND);
			break;
		case BLEND_MODE_MASK:
			Desc.Disable(EPSO_BLEND);
			break;
		case BLEND_MODE_ADDITIVE:
			Desc.Enable(EPSO_BLEND);
			Desc.BlendState.DestBlend = EBF_ONE;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
	}

	void TMaterial::SetShaderVsFormat(uint32 InVsFormat)
	{
		Desc.VsFormat = InVsFormat;
	}

	void TMaterial::EnableDepthWrite(bool bEnable)
	{
		if (bEnable)
		{
			Desc.Enable(EPSO_DEPTH);
		}
		else
		{
			Desc.Disable(EPSO_DEPTH);
		}
	}

	void TMaterial::EnableDepthTest(bool bEnable)
	{
		if (bEnable)
		{
			Desc.Enable(EPSO_DEPTH_TEST);
		}
		else
		{
			Desc.Disable(EPSO_DEPTH_TEST);
		}
	}

	void TMaterial::EnableTwoSides(bool bEnable)
	{
		Desc.RasterizerDesc.CullMode = bEnable ? ECM_NONE : ECM_BACK;
	}

	void TMaterial::EnableState(E_PIPELINE_STATES_OPTION InState, bool bEnable)
	{
		if (bEnable)
			Desc.Enable(InState);
		else
			Desc.Disable(InState);
	}

	void TMaterial::SetBlendState(const TBlendState& InBlendState)
	{
		Desc.BlendState = InBlendState;
	}

	void TMaterial::SetRasterizerState(const TRasterizerDesc& InRasterizerState)
	{
		Desc.RasterizerDesc = InRasterizerState;
	}

	void TMaterial::SetDepthStencilState(const TDepthStencilDesc& InDepthStencilState)
	{
		Desc.DepthStencilDesc = InDepthStencilState;
	}

	void TMaterial::SetRTColorBufferCount(int32 Count)
	{
		Desc.RTCount = Count;
	}

	void TMaterial::SetRTColor(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBuffer)
	{
		Desc.RTFormats[ColorBuffer] = Format;
	}
	
	void TMaterial::SetRTDepth(E_PIXEL_FORMAT Format)
	{
		Desc.DepthFormat = Format;
	}
}
