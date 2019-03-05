/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

static const int32 TriCount = 1024;
BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FTriangleInstanceBuffer, TriCount)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Velocity)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Offset)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Color)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, Projection)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(FFloat4, Padding, [9])
END_UNIFORM_BUFFER_STRUCT(FTriangleInstanceBuffer)

// Data structure to match the command signature used for ExecuteIndirect.
// Temp
BEGIN_UNIFORM_BUFFER_STRUCT_ARRAY(FIndirectCommandsList, TriCount)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(uint64, CBV)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FInt4, DrawArguments)
END_UNIFORM_BUFFER_STRUCT(FIndirectCommandsList)

BEGIN_UNIFORM_BUFFER_STRUCT(FComputeBuffer)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)
END_UNIFORM_BUFFER_STRUCT(FComputeBuffer)

BEGIN_UNIFORM_BUFFER_STRUCT(FResetBuffer)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(FInt4, ResetData, [16])
END_UNIFORM_BUFFER_STRUCT(FResetBuffer)

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

	FTriangleInstanceBufferPtr InstanceParamBuffer;
	FComputeBufferPtr ComputeBuffer;
	FIndirectCommandsListPtr IndirectCommandsBuffer;

	FUniformBufferPtr ProcessedCommandsBuffer;

	FResetBufferPtr ResetBuffer;

	FRenderResourceTablePtr ResourceTable;
};
