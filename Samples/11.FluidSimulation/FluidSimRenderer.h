/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FFluidSolver;
class FFluidSimRenderer : public FDefaultRenderer
{
public:
	static bool PauseUpdate;
	static bool StepNext;

	FFluidSimRenderer();
	virtual ~FFluidSimRenderer();

	static FFluidSimRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

	void MoveBoundary(const vector3df& offset);

private:

private:
	aabbox3df FluidBoundary;
	FMeshBufferPtr MB_Fluid;
	FPipelinePtr PL_Fluid;

	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FFluidSolver* Solver;
};
