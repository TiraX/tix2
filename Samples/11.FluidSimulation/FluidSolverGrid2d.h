/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"
#include "CurlCS.h"
#include "VorticityCS.h"
#include "DivergenceCS.h"
#include "ClearPressureCS.h"
#include "PressureCS.h"
#include "GradientSubstractCS.h"
#include "AdvectionVelCS.h"
#include "AdvectionDyeCS.h"
#include "SplatVelCS.h"
#include "SplatDyeCS.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FGrid2dParam)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info0)		// x = Dt; y = Curl; z = SimRes; w = DyeRes
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info1)		// x = PressureFade; y = VelDissipation; z = InvSimRes; w = InvDyeRes
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info2)		// xyz = DyeColor; w = DyeDissipation;
END_UNIFORM_BUFFER_STRUCT(FGrid2dParam)

class FFluidSolverGrid2d : public FFluidSolver
{
public:
	static const int32 SimResolution;
	static const int32 DyeResolution;
	static const float DensityDissipation;
	static const float VelocityDissipation;
	static const float Pressure;
	static const int32 PressureIterations;
	static const int32 Curl;
	static const float Radius;
	static const float SplatForce;

	FFluidSolverGrid2d();
	virtual ~FFluidSolverGrid2d();

	virtual void CreateGrid(FRHI* RHI, FFullScreenRender* FSRender) override;
	virtual void Sim(FRHI* RHI, float Dt) override;
	virtual void RenderGrid(FRHI* RHI, FScene* Scene, FFullScreenRender* FSRenderer) override;
	virtual void UpdateMousePosition(const vector2di& InPosition) override;

private:

private:
	// Compute Tasks
	FCurlCSPtr CurlCS;
	FVorticityCSPtr VorticityCS;
	FDivergenceCSPtr DivergenceCS;
	FClearPressureCSPtr ClearPressureCS;
	FPressureCSPtr PressureCS;
	FGradientSubstractCSPtr GradientSubstractCS;
	FAdvectionVelCSPtr AdvectionVelCS;
	FAdvectionDyeCSPtr AdvectionDyeCS;
	FSplatVelCSPtr SplatVelCS;
	FSplatDyeCSPtr SplatDyeCS;

	// Param Uniforms
	FGrid2dParamPtr UB_Grid2dParam;
	vector2di LastMousePos;
	vector2di CurrMousePos;
	vector2di MouseMoveDelta;

	// Resources
	FArgumentBufferPtr AB_DyeToScreen;
	FTexturePtr TexVelocity[2];
	FTexturePtr TexDye[2];
	FTexturePtr TexDivergence;
	FTexturePtr TexCurl;
	FTexturePtr TexPressure[2];
};
