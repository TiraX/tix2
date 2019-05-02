/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FUVDiscardInput)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)
END_UNIFORM_BUFFER_STRUCT(FUVDiscardInput)

class FComputeUVDiscard : public FComputeTask
{
public:
	static const int32 ThreadBlockSize = 8;

	FComputeUVDiscard(int32 W, int32 H);
	virtual ~FComputeUVDiscard();

	virtual void Run(FRHI * RHI) override;

	void PrepareBuffers(FTexturePtr UVInput);
protected:
	virtual void FinalizeInRenderThread() override;

private:
	vector2di InputSize;
	FUVDiscardInputPtr InputInfoBuffer;

	FUniformBufferPtr OutputUVBuffer;
	FRenderResourceTablePtr ResourceTable;
};
typedef TI_INTRUSIVE_PTR(FComputeUVDiscard) FComputeUVDiscardPtr;

////////////////////////////////////////////////////////

class FVirtualTextureRenderer : public FDefaultRenderer
{
public:
	FVirtualTextureRenderer();
	virtual ~FVirtualTextureRenderer();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;
	FArgumentBufferPtr AB_Result;

	FComputeUVDiscardPtr ComputeUVDiscard;
};
