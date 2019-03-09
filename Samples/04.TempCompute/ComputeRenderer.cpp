/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ComputeRenderer.h"
//Temp
#include "Dx12/d3dx12.h"
using namespace Microsoft::WRL;
#include "Dx12/FUniformBufferDx12.h"
#include "Dx12/FRHIDx12.h"
// link libraries
#pragma comment (lib, "d3d12.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

// Get a random float value between min and max.
inline float GetRandomFloat(float min, float max)
{
	const float inv = 1.f / RAND_MAX;
	float scale = static_cast<float>(rand()) * inv;
	float range = max - min;
	return scale * range + min;
}

// Test things
ComPtr<ID3D12CommandSignature> CommandSignature;

FComputeRenderer::FComputeRenderer()
{
	ComputeTask = FRHI::Get()->CreateComputeTask("S_TriangleCullCS");


	const TString TriangleMaterialName = "M_Triangle.tres";
	TMaterialPtr M_Triangle = static_cast<TMaterial*>(TResourceLibrary::Get()->LoadResource(TriangleMaterialName).get());
	PL_Triangle = M_Triangle->PipelineResource;
}

FComputeRenderer::~FComputeRenderer()
{
	ComputeTask = nullptr;

	CommandSignature = nullptr;
}

void FComputeRenderer::InitInRenderThread()
{
	FRHI * RHI = FRHI::Get();

	// Init resources
	static const float TriangleScale = 0.1f;
	static const vector3df TriangleVertices[3] = 
	{
		vector3df(-1.f * TriangleScale, -1.f * TriangleScale, 0.f),
		vector3df(1.f * TriangleScale, -1.f * TriangleScale, 0.f),
		vector3df(0.f * TriangleScale, 1.f * TriangleScale, 0.f)
	};
	static const uint16 TriangleIndices[3] = {
		0, 2, 1
	};

	TMeshBufferPtr MBData = ti_new TMeshBuffer();
	MBData->SetResourceName("TriangleTix");
	MBData->SetVertexStreamData(EVSSEG_POSITION, TriangleVertices, 3, EIT_16BIT, TriangleIndices, 3);
	TriangleMesh = RHI->CreateMeshBuffer();
	TriangleMesh->SetFromTMeshBuffer(MBData);
	RHI->UpdateHardwareResource(TriangleMesh, MBData);
	MBData = nullptr;

	// Init Constant Buffer
	ComputeBuffer = ti_new FComputeBuffer;
	ComputeBuffer->UniformBufferData[0].Info.X = 0.05f;	//TriangleHalfWidth
	ComputeBuffer->UniformBufferData[0].Info.Y = 1.f;		//TriangleDepth
	ComputeBuffer->UniformBufferData[0].Info.Z = 0.5f;		//CullingCutoff
	ComputeBuffer->UniformBufferData[0].Info.W = TriCount;	//TriangleCount;
	ComputeBuffer->InitUniformBuffer();

	// Draw instance parameters
	InstanceParamBuffer = ti_new FTriangleInstanceBuffer;
	for (int32 i = 0; i < TriCount; ++i)
	{
		InstanceParamBuffer->UniformBufferData[i].Velocity = FFloat4(GetRandomFloat(0.01f, 0.02f), 0.0f, 0.0f, 0.0f);
		InstanceParamBuffer->UniformBufferData[i].Offset = FFloat4(GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(0.0f, 2.0f), 0.0f);
		InstanceParamBuffer->UniformBufferData[i].Color = FFloat4(GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), 1.0f);
	}
	InstanceParamBuffer->InitUniformBuffer();

	// Indirect commands buffer
	IndirectCommandsBuffer = ti_new FIndirectCommandsList;
	FUniformBufferDx12* UBDx12 = static_cast<FUniformBufferDx12*>(InstanceParamBuffer->UniformBuffer.get());
	D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = UBDx12->GetConstantBuffer()->GetGPUVirtualAddress();
	for (int32 i = 0; i < TriCount; ++i)
	{
		IndirectCommandsBuffer->UniformBufferData[i].CBV = gpuAddress;
		IndirectCommandsBuffer->UniformBufferData[i].DrawArguments.X = 3;	//VertexCountPerInstance
		IndirectCommandsBuffer->UniformBufferData[i].DrawArguments.Y = 1;	//InstanceCount
		IndirectCommandsBuffer->UniformBufferData[i].DrawArguments.Z = 0;	//StartVertexLocation
		IndirectCommandsBuffer->UniformBufferData[i].DrawArguments.X = 0;	//StartInstanceLocation
		gpuAddress += InstanceParamBuffer->GetStructureStrideInBytes();
	}
	IndirectCommandsBuffer->InitUniformBuffer();

	// Create writable buffer for compute shader
	ProcessedCommandsBuffer = FRHI::Get()->CreateUniformBuffer(IndirectCommandsBuffer->GetStructureStrideInBytes(), IndirectCommandsBuffer->GetElementsSize(), UB_FLAG_COMPUTE_WRITABLE);
	RHI->UpdateHardwareResource(ProcessedCommandsBuffer, nullptr);

	// Reset Buffer
	ResetBuffer = ti_new FResetBuffer;
	for (int32 i = 0; i < 16; ++i)
	{
		ResetBuffer->UniformBufferData[0].ResetData[i] = FInt4();
	}
	ResetBuffer->InitUniformBuffer();

	// Create Render Resource Table
	ResourceTable = FRHI::Get()->CreateRenderResourceTable(3, EHT_SHADER_RESOURCE);
	ResourceTable->PutBufferInTable(InstanceParamBuffer->UniformBuffer, 0);
	ResourceTable->PutBufferInTable(IndirectCommandsBuffer->UniformBuffer, 1);
	ResourceTable->PutBufferInTable(ProcessedCommandsBuffer, 2);

	ComputeTask->Finalize();

	ComputeTask->SetConstantBuffer(0, ComputeBuffer->UniformBuffer);
	ComputeTask->SetParameter(1, ResourceTable);


	FRHIDx12 * RHIDx12 = static_cast<FRHIDx12*>(RHI);

	// Each command consists of a CBV update and a DrawInstanced call.
	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[2] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	argumentDescs[0].ConstantBufferView.RootParameterIndex = 0;
	argumentDescs[1].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;

	struct IndirectCommand
	{
		D3D12_GPU_VIRTUAL_ADDRESS cbv;
		D3D12_DRAW_ARGUMENTS drawArguments;
	};
	D3D12_COMMAND_SIGNATURE_DESC commandSignatureDesc = {};
	commandSignatureDesc.pArgumentDescs = argumentDescs;
	commandSignatureDesc.NumArgumentDescs = _countof(argumentDescs);
	commandSignatureDesc.ByteStride = sizeof(IndirectCommand);

	FRootSignatureDx12 * RS = static_cast<FRootSignatureDx12*>(PL_Triangle->GetShader()->ShaderBinding.get());
	VALIDATE_HRESULT(RHIDx12->GetD3DDevice()->CreateCommandSignature(&commandSignatureDesc, RS->Get(), IID_PPV_ARGS(&CommandSignature)));
}

void FComputeRenderer::Render(FRHI* RHI, FScene* Scene)
{
	Scene->PrepareViewUniforms();
	// Do compute cull first
	const int32 ThreadBlockSize = 128;	// Should match the value in S_TriangleCullCS
	TI_TODO("Reset command count");
	ComputeTask->Run(RHI, uint32(TriCount / float(ThreadBlockSize)), 1, 1);

    RHI->BeginRenderToFrameBuffer();
	// Render triangles to screen


	FUniformBufferDx12 * CmdBuffer = static_cast<FUniformBufferDx12*>(ProcessedCommandsBuffer.get());
	FUniformBufferDx12 * AllCmdBuffer = static_cast<FUniformBufferDx12*>(IndirectCommandsBuffer->UniformBuffer.get());

	FRHIDx12 * RHIDx12 = static_cast<FRHIDx12*>(RHI);
	RHI->SetGraphicsPipeline(PL_Triangle);
	RHI->SetMeshBuffer(TriangleMesh);
	if (true)
	{
		RHIDx12->GetRenderCommandList()->ExecuteIndirect(CommandSignature.Get(),
			TriCount,
			CmdBuffer->GetConstantBuffer().Get(),
			0,
			CmdBuffer->GetConstantBuffer().Get(),
			IndirectCommandsBuffer->GetStructureStrideInBytes() * IndirectCommandsBuffer->GetElementsSize()
		);
	}
	else
	{
		RHIDx12->GetRenderCommandList()->ExecuteIndirect(CommandSignature.Get(),
			TriCount,
			AllCmdBuffer->GetConstantBuffer().Get(),
			0,
			nullptr,
			0
		);
	}
}
