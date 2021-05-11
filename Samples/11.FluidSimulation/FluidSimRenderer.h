/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FFluidSolver;
class FFluidSimRenderer : public FDefaultRenderer
{
public:
	static const int32 FFT_Size = 512;

	FFluidSimRenderer();
	virtual ~FFluidSimRenderer();

	static FFluidSimRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	void DrawParticles(FRHI* RHI, FScene* Scene);

private:
	FMeshBufferPtr MB_Fluid;
	FPipelinePtr PL_Fluid;

	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FFluidSolver* Solver;
};
