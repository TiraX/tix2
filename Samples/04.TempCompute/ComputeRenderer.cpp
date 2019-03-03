/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ComputeRenderer.h"

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
	ComputeBuffer->UniformBufferData.Info.X = 0.05f;	//TriangleHalfWidth
	ComputeBuffer->UniformBufferData.Info.Y = 1.f;		//TriangleDepth
	ComputeBuffer->UniformBufferData.Info.Z = 0.5f;		//CullingCutoff
	ComputeBuffer->UniformBufferData.Info.W = TriCount;	//TriangleCount;
	ComputeBuffer->InitUniformBuffer();

	ConstantBuffer = ti_new FTriangleInstanceBuffer;
	for (int32 i = 0; i < TriCount; ++i)
	{
		ConstantBuffer->UniformBufferData.Buffer[i].velocity = FFloat4(GetRandomFloat(0.01f, 0.02f), 0.0f, 0.0f, 0.0f);
		ConstantBuffer->UniformBufferData.Buffer[i].offset = FFloat4(GetRandomFloat(-5.0f, -1.5f), GetRandomFloat(-1.0f, 1.0f), GetRandomFloat(0.0f, 2.0f), 0.0f);
		ConstantBuffer->UniformBufferData.Buffer[i].color = FFloat4(GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), GetRandomFloat(0.5f, 1.0f), 1.0f);
	}
	ConstantBuffer->InitUniformBuffer();

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
