/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

namespace tix
{
	TMaterial::TMaterial()
		: TResource(ERES_MATERIAL)
	{}

	TMaterial::~TMaterial()
	{
	}

	void TMaterial::InitRenderThreadResource()
	{
		// create pipeline
		TI_ASSERT(Pipeline == nullptr);
		Pipeline = ti_new TPipeline(*this);
		Pipeline->InitRenderThreadResource();

		// release shader codes
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			ShaderCodes[s].Destroy();
		}
	}

	void TMaterial::DestroyRenderThreadResource()
	{
		// destroy pipeline
		if (Pipeline != nullptr)
		{
			Pipeline->DestroyRenderThreadResource();
			Pipeline = nullptr;
		}
	}

	void TMaterial::SetShaderName(E_SHADER_STAGE Stage, const TString& Name)
	{
		ShaderNames[Stage] = Name;
	}

	void TMaterial::SetShaderCode(E_SHADER_STAGE Stage, const uint8* CodeBuffer, int32 Length)
	{
		ShaderCodes[Stage].Reset();
		ShaderCodes[Stage].Put(CodeBuffer, Length);
	}

	void TMaterial::SetBlendMode(E_BLEND_MODE InBlendMode)
	{
		BlendMode = InBlendMode;
	}

	void TMaterial::SetShaderVsFormat(uint32 InVsFormat)
	{
		VsFormat = InVsFormat;
	}

	void TMaterial::EnableDepthWrite(bool bEnable)
	{
		bDepthWrite = bEnable;
	}

	void TMaterial::EnableDepthTest(bool bEnable)
	{
		bDepthTest = bEnable;
	}

	void TMaterial::EnableTwoSides(bool bEnable)
	{
		bTwoSides = bEnable;
	}

	void TMaterial::SetRTColorBufferCount(int32 Count)
	{
		RTInfo.NumRT = Count;
	}

	void TMaterial::SetRTColor(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBuffer)
	{
		RTInfo.ColorRT[ColorBuffer] = Format;
	}
	
	void TMaterial::SetRTDepth(E_PIXEL_FORMAT Format)
	{
		RTInfo.DepthRT = Format;
	}
}
