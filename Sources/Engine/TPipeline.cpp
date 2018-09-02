/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TPipeline.h"

namespace tix
{
	TPipeline::TPipeline()
		: TResource(ERES_PIPELINE)
	{
	}

	TPipeline::TPipeline(const TMaterial& InMaterial)
		: TResource(ERES_PIPELINE)
	{
		// Change pipeline desc from InMaterial and InMesh
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			Desc.ShaderName[s] = InMaterial.ShaderNames[s];
			ShaderCode[s].Put(InMaterial.ShaderCodes[s].GetBuffer(), InMaterial.ShaderCodes[s].GetLength());
		}
		switch (InMaterial.BlendMode)
		{
		case TMaterial::MATERIAL_BLEND_OPAQUE:
			break;
		case TMaterial::MATERIAL_BLEND_TRANSLUCENT:
			Desc.Enable(EPSO_BLEND);
			break;
		case TMaterial::MATERIAL_BLEND_MASK:
			break;
		case TMaterial::MATERIAL_BLEND_ADDITIVE:
			Desc.Enable(EPSO_BLEND);
			Desc.BlendState.DestBlend = EBF_ONE;
			break;
		default:
			TI_ASSERT(0);
			break;
		}
		if (!InMaterial.bDepthWrite)
			Desc.Disable(EPSO_DEPTH);
		if (!InMaterial.bDepthTest)
			Desc.Disable(EPSO_DEPTH_TEST);
		if (InMaterial.bTwoSides)
			Desc.RasterizerDesc.CullMode = ECM_NONE;

		Desc.VsFormat = InMaterial.VsFormat;

		// if Material.RTInfo.NumRT > 0, use material's rt info;
		// else use default frame buffer info
		if (InMaterial.RTInfo.NumRT > 0)
		{
			Desc.RTCount = InMaterial.RTInfo.NumRT;
			for (int32 c = 0; c < Desc.RTCount; ++c)
			{
				Desc.RTFormats[c] = InMaterial.RTInfo.ColorRT[c];
			}
			Desc.DepthFormat = InMaterial.RTInfo.DepthRT;
		}
	}

	TPipeline::~TPipeline()
	{
	}

	void TPipeline::SetShader(E_SHADER_STAGE ShaderStage, const TString& ShaderName, const int8* InShaderCode, int32 CodeLength)
	{
		Desc.ShaderName[ShaderStage] = ShaderName;
		ShaderCode[ShaderStage].Put(InShaderCode, CodeLength);
	}
	
	void TPipeline::InitRenderThreadResource()
	{
		TI_ASSERT(PipelineResource == nullptr);
		PipelineResource = FRHI::Get()->CreatePipeline();

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(TPipelineUpdateResource,
			FPipelinePtr, Pipeline_RT, PipelineResource,
			TPipelinePtr, PipeDesc, this,
			{
				RHI->UpdateHardwareResource(Pipeline_RT, PipeDesc);
			});
	}

	void TPipeline::DestroyRenderThreadResource()
	{
		TI_ASSERT(PipelineResource != nullptr);

		ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(TPipelineDestroyFPipeline,
			FPipelinePtr, Pipeline_RT, PipelineResource,
			{
				Pipeline_RT->Destroy();
			});
		PipelineResource = nullptr;
	}
}
