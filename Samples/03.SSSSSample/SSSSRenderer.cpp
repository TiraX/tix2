/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SSSSRenderer.h"

FSSSSRenderer::FSSSSRenderer()
{
}

FSSSSRenderer::~FSSSSRenderer()
{
	RTTest = nullptr;
}

void FSSSSRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();
	FSRender.InitCommonResources(RHI);

	// Render target test case
	RTTest = FRenderTarget::Create(1280, 720);
#if defined (TIX_DEBUG)
	RTTest->SetResourceName("RTTest");
#endif
	RTTest->AddColorBuffer(EPF_BGRA8, ERTC_COLOR0);
	RTTest->AddDepthStencilBuffer(EPF_DEPTH24_STENCIL8);
	RTTest->Compile();
}

void FSSSSRenderer::Render(FRHI* RHI, FScene* Scene)
{
	PrepareViewUniforms(Scene);

	RHI->PushRenderTarget(RTTest);

	const TVector<FPrimitivePtr>& Primitives = Scene->GetStaticDrawList();
	for (const auto& Primitive : Primitives)
	{
		for (int32 m = 0; m < (int32)Primitive->MeshBuffers.size(); ++m)
		{
			FMeshBufferPtr MB = Primitive->MeshBuffers[m];
			FPipelinePtr PL = Primitive->Pipelines[m];
			FUniformBufferPtr UB = Primitive->Uniforms[m];
			FRenderResourceTablePtr TextureTable = Primitive->TextureTables[m];

			{
				//D3D12 ERROR : ID3D12CommandQueue::ExecuteCommandLists : 
				//Specified GPU Descriptor Handle(ptr = 0x264d1eae200 at 45 offsetInDescriptorsFromDescriptorHeapStart), 
				//for Root Signature(0x00000264D205CAC0:'RootSignature')'s Descriptor Table (at Parameter Index [2])'s 
				//Descriptor Range(at Range Index[0] of type D3D12_DESCRIPTOR_RANGE_TYPE_SRV) has not been initialized,
				//at Draw Index : [1].On Resource Binding Tier 1 hardware, 
				//all descriptor tables declared in the set Root Signature must be populated and initialized, 
				//even if the shaders do not need the descriptor.[EXECUTION ERROR #646: INVALID_DESCRIPTOR_HANDLE]
			}

			{
				RHI->SetMeshBuffer(MB);
				RHI->SetPipeline(PL);
				RHI->SetUniformBuffer(0, ViewUniformBuffer->UniformBuffer);
				RHI->SetUniformBuffer(1, Primitive->LightBindingUniformBuffer->UniformBuffer);

				RHI->SetDynamicLightsUniformBuffer();

				if (TextureTable != nullptr)
				{
					RHI->SetRenderResourceTable(3, TextureTable);
				}
				RHI->DrawPrimitiveIndexedInstanced(MB->GetIndicesCount(), 1, 0, 0, 0);
			}
		}
	}
	
	RHI->PopRenderTarget();

	FSRender.DrawFullScreenTexture(RHI, RTTest->GetColorBuffer(ERTC_COLOR0).Texture);
}