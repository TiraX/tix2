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

// Get a random float value between min and max.
inline float GetRandomFloat(float min, float max)
{
	const float inv = 1.f / RAND_MAX;
	float scale = static_cast<float>(rand()) * inv;
	float range = max - min;
	return scale * range + min;
}

FComputeRenderer::FComputeRenderer()
{
	ComputeTask = FRHI::Get()->CreateComputeTask("S_TriangleCullCS");
}

FComputeRenderer::~FComputeRenderer()
{
	ComputeTask = nullptr;
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
		InstanceParamBuffer->UniformBufferData[i].Offset = FFloat4(GetRandomFloat(-5.0f, -1.5f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(0.0f, 2.0f), 0.0f);
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
	TI_ASSERT(0);

	// Create Render Resource Table
	ResourceTable = FRHI::Get()->CreateRenderResourceTable(3, EHT_SHADER_RESOURCE);
	ResourceTable->PutBufferInTable(InstanceParamBuffer->UniformBuffer, 0);
	ResourceTable->PutBufferInTable(IndirectCommandsBuffer->UniformBuffer, 1);

	ComputeTask->Finalize();
}

void FComputeRenderer::Render(FRHI* RHI, FScene* Scene)
{
	Scene->PrepareViewUniforms();
	// Do compute cull first
	// 1, set parameter

    RHI->BeginRenderToFrameBuffer();
	// Render triangles to screen
}
