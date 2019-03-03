/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once


struct SceneConstantBuffer
{
	FFloat4 velocity;
	FFloat4 offset;
	FFloat4 color;
	FMatrix projection;

	// Constant buffers are 256-byte aligned. Add padding in the struct to allow multiple buffers
	// to be array-indexed.
	float padding[36];
};

static const int32 TriCount = 1024;
BEGIN_UNIFORM_BUFFER_STRUCT(FTriangleInstanceBuffer)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(SceneConstantBuffer, Buffer, [TriCount])
END_UNIFORM_BUFFER_STRUCT(FTriangleInstanceBuffer)

BEGIN_UNIFORM_BUFFER_STRUCT(FComputeBuffer)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)
END_UNIFORM_BUFFER_STRUCT(FComputeBuffer)

class FComputeRenderer : public FDefaultRenderer
{
public:
	FComputeRenderer();
	virtual ~FComputeRenderer();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
protected:
	FComputeTaskPtr ComputeTask;
	FMeshBufferPtr TriangleMesh;

	FComputeBufferPtr ComputeBuffer;
	FTriangleInstanceBufferPtr ConstantBuffer;
};
