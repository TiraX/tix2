/*
	TiX Engine v2.0 Copyright (C) 2018~2019
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
		FullScreenShader = nullptr;
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
		RHI->UpdateHardwareResourceMesh(FullScreenQuad, MBData);
		MBData = nullptr;

		// Create full screen render pipeline
		TMaterialPtr FSMaterial = ti_new TMaterial();
		FSMaterial->SetResourceName("FullScreenMaterial");

		TShaderNames ShaderNames;
		ShaderNames.ShaderNames[ESS_VERTEX_SHADER] = "FullScreenVS";
		ShaderNames.ShaderNames[ESS_PIXEL_SHADER] = "FullScreenPS";

		// Move this TShader load to Game Thread. Or Make a gloal shader system.
		TShaderPtr Shader = ti_new TShader(ShaderNames);
		Shader->LoadShaderCode();
		Shader->ShaderResource = RHI->CreateShader(ShaderNames);
		RHI->UpdateHardwareResourceShader(Shader->ShaderResource, Shader);
		FSMaterial->SetShader(Shader);
		FullScreenShader = Shader->ShaderResource;

        FSMaterial->EnableTwoSides(true);
		FSMaterial->EnableDepthWrite(false);
		FSMaterial->EnableDepthTest(false);
		FSMaterial->SetShaderVsFormat(FullScreenQuad->GetVSFormat());
		FSMaterial->SetRTColor(FRHIConfig::DefaultBackBufferFormat, ERTC_COLOR0);

		// Pipeline
		FullScreenPipeline = RHI->CreatePipeline(FullScreenShader);
		RHI->UpdateHardwareResourcePL(FullScreenPipeline, FSMaterial);
		FSMaterial = nullptr;

		bInited = true;
	}

	void FFullScreenRender::DrawFullScreenTexture(FRHI* RHI, FTexturePtr Texture)
	{
		TI_ASSERT(bInited);
        RHI->SetGraphicsPipeline(FullScreenPipeline);
        RHI->SetMeshBuffer(FullScreenQuad, nullptr);
        RHI->SetShaderTexture(0, Texture);
        
        RHI->DrawPrimitiveIndexedInstanced(FullScreenQuad, 1);
	}

	void FFullScreenRender::DrawFullScreenTexture(FRHI* RHI, FRenderResourceTablePtr TextureTable)
	{
		TI_ASSERT(bInited);
		RHI->SetMeshBuffer(FullScreenQuad, nullptr);
		RHI->SetGraphicsPipeline(FullScreenPipeline);
		RHI->SetRenderResourceTable(0, TextureTable);

		RHI->DrawPrimitiveIndexedInstanced(FullScreenQuad, 1);
	}

	void FFullScreenRender::DrawFullScreenTexture(FRHI* RHI, FArgumentBufferPtr ArgumentBuffer)
	{
		TI_ASSERT(bInited);
        RHI->SetGraphicsPipeline(FullScreenPipeline);
		RHI->SetMeshBuffer(FullScreenQuad, nullptr);
		RHI->SetArgumentBuffer(ArgumentBuffer);

		RHI->DrawPrimitiveIndexedInstanced(FullScreenQuad, 1);
	}

	void FFullScreenRender::DrawFullScreenQuad(FRHI* RHI)
	{
		TI_ASSERT(bInited);
		RHI->SetMeshBuffer(FullScreenQuad, nullptr);
		RHI->DrawPrimitiveIndexedInstanced(FullScreenQuad, 1);
	}
}
