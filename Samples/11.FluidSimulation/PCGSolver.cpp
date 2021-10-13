/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "PCGSolver.h"
#include "FluidSimRenderer.h"


#define DO_PARALLEL (0)
const float eps = 1e-9f;


FPCGSolver::FPCGSolver()
{
}

FPCGSolver::~FPCGSolver()
{
}

void FPCGSolver::Solve(PCGSolverParameters& Parameter)
{

}

void FPCGSolver::CalcNegativeDivergence(const FFluidGrid3<float>& U, const FFluidGrid3<float>& V, const FFluidGrid3<float>& W)
{
//	TI_ASSERT(Divergence.GetDimension() == Pressure.GetDimension());
//#if DO_PARALLEL
//#pragma omp parallel for
//#endif
//	for (int32 Index = 0; Index < Divergence.GetTotalCells(); Index++)
//	{
//		vector3di GridIndex = Divergence.ArrayIndexToGridIndex(Index);
//
//		float ULeft = VelField[0].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
//		float URight = VelField[0].SafeCell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
//		float VFront = VelField[1].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
//		float VBack = VelField[1].SafeCell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
//		float WUp = VelField[2].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z);
//		float WDown = VelField[2].SafeCell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);
//
//		//float L = (GridIndex.X == 0) ? -V.X : VLeft.X;
//		//float R = (GridIndex.X == (Dimension.X - 1)) ? -V.X : VRight.X;
//		//float F = (GridIndex.Y == 0) ? -V.Y : VFront.Y;
//		//float B = (GridIndex.Y == (Dimension.Y - 1)) ? -V.Y : VBack.Y;
//		//float U = (GridIndex.Z == 0) ? -V.Z : VUp.Z;
//		//float D = (GridIndex.Z == (Dimension.Z - 1)) ? -V.Z : VDown.Z;
//		float L = ULeft;
//		float R = URight;
//		float F = VFront;
//		float B = VBack;
//		float U = WUp;
//		float D = WDown;
//
//		float Div = (R - L) * InvCellSize.X + (B - F) * InvCellSize.Y + (D - U) * InvCellSize.Z;
//		Divergence.Cell(Index) = Div;
//	}
}