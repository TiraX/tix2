/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FTileDeterminationCS : public FComputeTask
{
public:
	static const int32 ThreadBlockSize = 32;

	FTileDeterminationCS(int32 W, int32 H);
	virtual ~FTileDeterminationCS();

	virtual void Run(FRHI * RHI) override;

	void PrepareBuffers(FTexturePtr UVInput);
	void PrepareDataForCPU(FRHI * RHI);
	TStreamPtr ReadUVBuffer();
	void ClearQuadTree(FRHI * RHI);
protected:

private:
	vector2di InputSize;

	FUniformBufferPtr QuadTreeBufferClear;
	FUniformBufferPtr QuadTreeBuffer;
	FArgumentBufferPtr ComputeArgument;

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
