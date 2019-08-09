/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FUVDiscardInput)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)
END_UNIFORM_BUFFER_STRUCT(FUVDiscardInput)

class FTileDeterminationCS : public FComputeTask
{
public:
	static const int32 ThreadBlockSize = 32;

	FTileDeterminationCS(int32 W, int32 H);
	virtual ~FTileDeterminationCS();

	virtual void Run(FRHI * RHI) override;

	void PrepareBuffers(FTexturePtr UVInput);
	void PrepareDataForCPU(FRHI * RHI, int32 FrameNum);
	TStreamPtr ReadUVBuffer();
protected:
	virtual void FinalizeInRenderThread() override;

private:
	vector2di InputSize;
	FUVDiscardInputPtr InputInfoBuffer;

	FUniformBufferPtr QuadTreeBuffer;
	FRenderResourceTablePtr ResourceTable;

	bool UVBufferTriggerd;
};
typedef TI_INTRUSIVE_PTR(FTileDeterminationCS) FTileDeterminationCSPtr;

////////////////////////////////////////////////////////

enum E_Graphics_Pipeline
{
	GraphicsBasePass,
	GraphicsBlit,

	GraphicsCount
};

enum E_Compute_Pipeline
{
	ComputeTileDetermine,

	ComputeCount
};

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

	FTileDeterminationCSPtr ComputeTileDetermination;
};
