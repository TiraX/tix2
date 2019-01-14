/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRenderer.h"
#include "FFullScreenRender.h"

namespace tix
{
	FFullScreenRender::FFullScreenRender()
		: bInited(false)
	{}

	FFullScreenRender::~FFullScreenRender()
	{
		FullScreenQuad = nullptr;
		FullScreenPipeline = nullptr;
		FullScreenBinding = nullptr;
	}

	void FFullScreenRender::InitCommonResources(FRHI* RHI)
	{
		if (bInited)
			return;

		// Create full screen quad
		const float16 half0 = float16(0.f);
		const float16 half1 = float16(1.f);
		static const FullScreenVertex FullScreenQuadVertices[4] = {
			{vector3df(-1.f, -1.f, 0.f), vector2df16(half0, half1)},
			{vector3df(1.f, -1.f, 0.f), vector2df16(half1, half1)},
			{vector3df(-1.f, 1.f, 0.f), vector2df16(half0, half0)},
			{vector3df(1.f, 1.f, 0.f), vector2df16(half1, half0)}
		};
		static const uint16 FullScreenQuadIndices[6] = {
			0, 2, 1, 1, 2, 3
		};

		TMeshBufferPtr MBData = ti_new TMeshBuffer();
		MBData->SetResourceName("FullScreenQuad");
		MBData->SetVertexStreamData(EVSSEG_POSITION | EVSSEG_TEXCOORD0, FullScreenQuadVertices, 4, EIT_16BIT, FullScreenQuadIndices, 6);
		FullScreenQuad = RHI->CreateMeshBuffer();
		FullScreenQuad->SetFromTMeshBuffer(MBData);
		RHI->UpdateHardwareResource(FullScreenQuad, MBData);
		MBData = nullptr;

		// Create full screen shader binding
		FShaderBindingPtr FullScreenBinding = RHI->CreateShaderBinding(1);
		FullScreenBinding->InitBinding(0, BINDING_TEXTURE_TABLE, 0, 1, ESS_PIXEL_SHADER);
		FSamplerDesc Desc;
		Desc.Filter = ETFT_MINMAG_LINEAR_MIP_NEAREST;
		Desc.AddressMode = ETC_CLAMP_TO_EDGE;
		FullScreenBinding->InitStaticSampler(0, Desc, ESS_PIXEL_SHADER);
		FullScreenBinding->Finalize(RHI);

		// Create full screen render pipeline
		TMaterialPtr FSMaterial = ti_new TMaterial();
		FSMaterial->SetResourceName("FullScreenMaterial");

		TShaderNames ShaderNames;
		ShaderNames.ShaderNames[ESS_VERTEX_SHADER] = "FullScreenVS";
		ShaderNames.ShaderNames[ESS_PIXEL_SHADER] = "FullScreenPS";

		TShaderPtr Shader = ti_new TShader(ShaderNames);
		Shader->ShaderResource = RHI->CreateShader(ShaderNames);
		RHI->UpdateHardwareResource(Shader->ShaderResource);
		FSMaterial->SetShader(Shader);

		FSMaterial->EnableDepthWrite(false);
		FSMaterial->EnableDepthTest(false);
		FSMaterial->SetShaderVsFormat(FullScreenQuad->GetVSFormat());
		FSMaterial->SetRTColor(FRHIConfig::DefaultBackBufferFormat, ERTC_COLOR0);

		FullScreenPipeline = RHI->CreatePipeline();
		FullScreenPipeline->SetShaderBinding(FullScreenBinding);
		RHI->UpdateHardwareResource(FullScreenPipeline, FSMaterial);
		FSMaterial = nullptr;

		bInited = true;
	}

	void FFullScreenRender::DrawFullScreenTexture(FRHI* RHI, FTexturePtr Texture)
	{
		TI_ASSERT(bInited);
		TI_ASSERT(0);
	}

	void FFullScreenRender::DrawFullScreenTexture(FRHI* RHI, FRenderResourceTablePtr TextureTable)
	{
		TI_ASSERT(bInited);
		RHI->SetMeshBuffer(FullScreenQuad);
		RHI->SetPipeline(FullScreenPipeline);
		RHI->SetRenderResourceTable(0, TextureTable);

		RHI->DrawPrimitiveIndexedInstanced(FullScreenQuad->GetIndicesCount(), 1, 0, 0, 0);
	}

	void FFullScreenRender::DrawFullScreenQuad(FRHI* RHI)
	{
		TI_ASSERT(bInited);
		RHI->SetMeshBuffer(FullScreenQuad);
		RHI->DrawPrimitiveIndexedInstanced(FullScreenQuad->GetIndicesCount(), 1, 0, 0, 0);
	}
}
