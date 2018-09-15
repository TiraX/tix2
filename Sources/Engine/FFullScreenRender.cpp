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
	}

	void FFullScreenRender::InitCommonResources(FRHI* RHI)
	{
		if (bInited)
			return;

		// create full screen quad
		static const FullScreenVertex FullScreenQuadVertices[4] = {
			{vector3df(-1.f, -1.f, 0.f), vector2df(0.f, 1.f)},
			{vector3df(1.f, -1.f, 0.f), vector2df(1.f, 1.f)},
			{vector3df(-1.f, 1.f, 0.f), vector2df(0.f, 0.f)},
			{vector3df(1.f, 1.f, 0.f), vector2df(1.f, 0.f)}
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

		// create full screen render pipeline
		TPipelinePtr Pipeline = ti_new TPipeline();
		Pipeline->SetResourceName("FullScreenPL");
		const TString ShaderPaths[ESS_COUNT] = {
			"FullScreenVS.cso",
			"FullScreenPS.cso",
			"",
			"",
			""
		};
		for (int32 s = 0; s < ESS_COUNT; ++s)
		{
			if (ShaderPaths[s].empty())
				continue;

			TFile f;
			if (f.Open(ShaderPaths[s], EFA_READ))
			{
				TStream Buffer;
				Buffer.Put(f);
				f.Close();
				Pipeline->SetShader((E_SHADER_STAGE)s, ShaderPaths[s], Buffer.GetBuffer(), Buffer.GetLength());
			}
			else
			{
				_LOG(Error, "Failed to load shader [%s].\n", ShaderPaths[s].c_str());
			}
		}
		Pipeline->Desc.Disable(EPSO_DEPTH);
		Pipeline->Desc.Disable(EPSO_DEPTH_TEST);
		Pipeline->Desc.VsFormat = FullScreenQuad->GetVSFormat();

		FullScreenPipeline = RHI->CreatePipeline();
		RHI->UpdateHardwareResource(FullScreenPipeline, Pipeline);
		Pipeline = nullptr;

		bInited = true;
	}

	void FFullScreenRender::DrawFullScreenTexture(FRHI* RHI, FTexturePtr Texture)
	{
		TI_ASSERT(bInited);
		RHI->SetMeshBuffer(FullScreenQuad);
		RHI->SetPipeline(FullScreenPipeline);
		RHI->SetShaderTexture(1, Texture);

		RHI->DrawPrimitiveIndexedInstanced(FullScreenQuad->GetIndicesCount(), 1, 0, 0, 0);
	}
}