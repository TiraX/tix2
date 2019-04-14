/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	TMaterial::TMaterial()
		: BlendMode(BLEND_MODE_OPAQUE)
	{}

	TMaterial::~TMaterial()
	{
	}

	void TMaterial::SetShaderVsFormat(uint32 InVsFormat)
	{
		Desc.VsFormat = InVsFormat;
	}

	void TMaterial::SetShaderInsFormat(uint32 InInsFormat)
	{
		Desc.InsFormat = InInsFormat;
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

	void TMaterial::SetBlendState(E_BLEND_MODE InBlendMode, const TBlendState& InBlendState)
	{
		BlendMode = InBlendMode;
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
        if (Format == EPF_DEPTH24_STENCIL8)
        {
            // For metal.
            Desc.StencilFormat = Format;
        }
	}
}
