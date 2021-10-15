/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolverFlipCPU.h"
#include "FluidSimRenderer.h"


#define DO_PARALLEL (0)

const float eps = 1e-9f;
const int32 UNKNOWN = 0x00;
const int32 WAITING = 0x01;
const int32 KNOWN = 0x02;
const int32 DONE = 0x03;


template < class T>
inline vector3d<T> ClampVector3d(const vector3d<T>& V, const vector3d<T>& Min, const vector3d<T>& Max)
{
	vector3d<T> Result;
	Result.X = TMath::Clamp(V.X, Min.X, Max.X);
	Result.Y = TMath::Clamp(V.Y, Min.Y, Max.Y);
	Result.Z = TMath::Clamp(V.Z, Min.Z, Max.Z);
	return Result;
}

template <class T>
inline vector3d<T> MinVector3d(const vector3d<T>& A, const vector3d<T>& B)
{
	vector3d<T> Result;
	Result.X = TMath::Min(A.X, B.X);
	Result.Y = TMath::Min(A.Y, B.Y);
	Result.Z = TMath::Min(A.Z, B.Z);
	return Result;
}

template <class T>
inline vector3d<T> MaxVector3d(const vector3d<T>& A, const vector3d<T>& B)
{
	vector3d<T> Result;
	Result.X = TMath::Max(A.X, B.X);
	Result.Y = TMath::Max(A.Y, B.Y);
	Result.Z = TMath::Max(A.Z, B.Z);
	return Result;
}

inline vector3di Floor(const vector3df& x)
{
	vector3di i;
	i.X = TMath::Floor(x.X);
	i.Y = TMath::Floor(x.Y);
	i.Z = TMath::Floor(x.Z);
	return i;
}

FFluidSolverFlipCPU::FFluidSolverFlipCPU()
{
	SubStep = 1;
}

FFluidSolverFlipCPU::~FFluidSolverFlipCPU()
{
}

void FFluidSolverFlipCPU::CreateParticles(
	const aabbox3df& InParticleBox,
	float InParticleSeperation,
	float InParticleMass)
{
	Particles.CreateParticlesInBox(InParticleBox, InParticleSeperation, InParticleMass);
	Flag |= DirtyParticles;
	Flag |= DirtyParams;
}

void FFluidSolverFlipCPU::CreateGrid(const vector3di& Dim)
{
	CellSize.X = BoundaryBox.getExtent().X / Dim.X;
	CellSize.Y = BoundaryBox.getExtent().Y / Dim.Y;
	CellSize.Z = BoundaryBox.getExtent().Z / Dim.Z;

	InvCellSize = vector3df(1.f) / CellSize;
	
	Dimension = Dim;
	VelField[0].Create(vector3di(Dim.X + 1, Dim.Y, Dim.Z));
	VelField[1].Create(vector3di(Dim.X, Dim.Y + 1, Dim.Z));
	VelField[2].Create(vector3di(Dim.X, Dim.Y, Dim.Z + 1));
	Marker.Create(Dim);
	Divergence.Create(Dim);
	Pressure.Create(Dim);
	VelFieldDelta[0].Create(VelField[0].GetDimension());
	VelFieldDelta[1].Create(VelField[1].GetDimension());
	VelFieldDelta[2].Create(VelField[2].GetDimension());
}

int32 Counter = 0;
void FFluidSolverFlipCPU::Sim(FRHI * RHI, float Dt)
{
	TIMER_RECORDER("FlipSim");
	AdvectVelocityField();
	ExtrapolateVelocityField();
	MarkCells();
	BackupVelocity();
	CalcExternalForces(Dt);
	CalcVisicosity(Dt);
	SolvePressure(Dt);
	ApplyPressure(Dt);
	ExtrapolateVelocityField();
	ConstrainVelocityField();
	AdvectParticles(Dt);
}

void FFluidSolverFlipCPU::AdvectVelocityField()
{
	// Clear vel and weight to 0
	VelField[0].Clear();
	VelField[1].Clear();
	VelField[2].Clear();

	const vector3df HalfCellSize = CellSize * 0.5f;

	vector3df VelStartPos[3];
	VelStartPos[0] = vector3df(0.f, HalfCellSize.Y, HalfCellSize.Z);
	VelStartPos[1] = vector3df(HalfCellSize.X, 0.f, HalfCellSize.Z);
	VelStartPos[2] = vector3df(HalfCellSize.X, HalfCellSize.Y, 0.f);

	FFluidGrid3<float> Weights[3];
	Weights[0].Create(VelField[0].GetDimension());
	Weights[1].Create(VelField[1].GetDimension());
	Weights[2].Create(VelField[2].GetDimension());

	float r[3], rsq[3], coef1[3], coef2[3], coef3[3];
	for (int32 i = 0; i < 3; i++)
	{
		r[i] = CellSize[i];
		rsq[i] = r[i] * r[i];
		coef1[i] = (4.f / 9.f) * (1.f / (rsq[i] * rsq[i] * rsq[i]));
		coef2[i] = (17.f / 9.f) * (1.f / (rsq[i] * rsq[i]));
		coef3[i] = (22.f / 9.f) * (1.f / rsq[i]);
	}

	// Advect U,V,W
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0 ; Index < Particles.GetTotalParticles(); Index ++)
	{
		for (int32 VelIndex = 0; VelIndex < 3; VelIndex++)
		{
			const vector3df& Vel = Particles.GetParticleVelocity(Index);
			vector3df PosInGrid = Particles.GetParticlePosition(Index) - VelStartPos[VelIndex];
			vector3di IndexInGrid = Floor(PosInGrid * InvCellSize);
			vector3di GridMin = vector3di(IndexInGrid.X - 1, IndexInGrid.Y - 1, IndexInGrid.Z - 1);
			vector3di GridMax = vector3di(IndexInGrid.X + 1, IndexInGrid.Y + 1, IndexInGrid.Z + 1);
			const vector3di& GridDim = VelField[VelIndex].GetDimension();
			GridMin = MaxVector3d(GridMin, vector3di());
			GridMax = MinVector3d(GridMax, vector3di(GridDim.X - 1, GridDim.Y - 1, GridDim.Z - 1));

			for (int32 k = GridMin.Z; k <= GridMax.Z; k++)
			{
				for (int32 j = GridMin.Y; j <= GridMax.Y; j++)
				{
					for (int32 i = GridMin.X; i <= GridMax.X; i++)
					{
						vector3df GridPos = vector3df(float(i), float(j), float(k)) * CellSize;
						vector3df DisVector = GridPos - PosInGrid;
						float DisSQ = DisVector.dotProduct(DisVector);
						if (DisSQ < rsq[VelIndex])
						{
							float Weight =
								1.f - coef1[VelIndex] * DisSQ * DisSQ * DisSQ +
								coef2[VelIndex] * DisSQ * DisSQ -
								coef3[VelIndex] * DisSQ;
#if DO_PARALLEL
#pragma omp critical
#endif
							{
								VelField[VelIndex].Cell(i, j, k) += Vel[VelIndex] * Weight;
								Weights[VelIndex].Cell(i, j, k) += Weight;
							}
						}
					}
				}
			}
		}
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		if (Weights[0].Cell(Index) > eps)
			VelField[0].Cell(Index) = VelField[0].Cell(Index) / Weights[0].Cell(Index);
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		if (Weights[1].Cell(Index) > eps)
			VelField[1].Cell(Index) = VelField[1].Cell(Index) / Weights[1].Cell(Index);
	}

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		if (Weights[2].Cell(Index) > eps)
			VelField[2].Cell(Index) = VelField[2].Cell(Index) / Weights[2].Cell(Index);
	}
}

void TryToMarkCell(FFluidGrid3<int32>& Status, TVector<vector3di>& ExtrapolateCells, int32& Count, const vector3di& GridIndex)
{
	if (Status.Cell(GridIndex) == UNKNOWN)
	{
		ExtrapolateCells.push_back(GridIndex);
		Status.Cell(GridIndex) = WAITING;
		Count++;
	}
	else if (Status.Cell(GridIndex) == WAITING)
	{
		Count++;
	}
}

void FFluidSolverFlipCPU::ExtrapolateVelocityField()
{
	const int32 ExtraLayers = 5;

	FFluidGrid3<int32> Status[3];
	Status[0].Create(VelField[0].GetDimension());
	Status[1].Create(VelField[1].GetDimension());
	Status[2].Create(VelField[2].GetDimension());

#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		vector3di G = VelField[0].ArrayIndexToGridIndex(Index);
		bool IsFluid = Marker.IsUMarkerValueEqual(Marker_Fluid, G.X, G.Y, G.Z);
		Status[0].Cell(Index) = IsFluid ? KNOWN : UNKNOWN;
		uint32 Info = VelField[0].GetBoudaryTypeInfo(Index);
		if (Status[0].Cell(Index) == UNKNOWN && Info != Boundary_None)
		{
			Status[0].Cell(Index) = DONE;
		}
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		vector3di G = VelField[1].ArrayIndexToGridIndex(Index);
		bool IsFluid = Marker.IsVMarkerValueEqual(Marker_Fluid, G.X, G.Y, G.Z);
		Status[1].Cell(Index) = IsFluid ? KNOWN : UNKNOWN;
		uint32 Info = VelField[1].GetBoudaryTypeInfo(Index);
		if (Status[1].Cell(Index) == UNKNOWN && Info != Boundary_None)
		{
			Status[1].Cell(Index) = DONE;
		}
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		vector3di G = VelField[2].ArrayIndexToGridIndex(Index);
		bool IsFluid = Marker.IsWMarkerValueEqual(Marker_Fluid, G.X, G.Y, G.Z);
		Status[2].Cell(Index) = IsFluid ? KNOWN : UNKNOWN;
		uint32 Info = VelField[2].GetBoudaryTypeInfo(Index);
		if (Status[2].Cell(Index) == UNKNOWN && Info != Boundary_None)
		{
			Status[2].Cell(Index) = DONE;
		}
	}

	TVector<vector3di> ExtrapolationCells;
	for (int layers = 0; layers < ExtraLayers; layers++) 
	{
		for (int32 VelIndex = 0; VelIndex < 3; VelIndex++)
		{
			ExtrapolationCells.clear();

#if DO_PARALLEL
#pragma omp parallel for
#endif
			for (int32 Index = 0; Index < VelField[VelIndex].GetTotalCells(); Index++)
			{
				vector3di GridIndex = VelField[VelIndex].ArrayIndexToGridIndex(Index);
				uint32 BoundaryInfo = VelField[VelIndex].GetBoudaryTypeInfo(Index);
				if (BoundaryInfo != Boundary_None || Status[VelIndex].Cell(Index) != KNOWN)
					continue;

				int32 Count = 0;
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X - 1, GridIndex.Y, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X + 1, GridIndex.Y, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y - 1, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y + 1, GridIndex.Z));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z - 1));
				TryToMarkCell(Status[VelIndex], ExtrapolationCells, Count, vector3di(GridIndex.X, GridIndex.Y, GridIndex.Z + 1));

				if (Count == 0)
				{
					Status[VelIndex].Cell(Index) = DONE;
				}
			}

			vector3di G;
			for (size_t i = 0; i < ExtrapolationCells.size(); i++) {
				G = ExtrapolationCells[i];

				float Sum = 0.f;
				int Count = 0;
				if (Status[VelIndex].Cell(G.X - 1, G.Y, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X - 1, G.Y, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X + 1, G.Y, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X + 1, G.Y, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y - 1, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y - 1, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y + 1, G.Z) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y + 1, G.Z); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y, G.Z - 1) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y, G.Z - 1); Count++; }
				if (Status[VelIndex].Cell(G.X, G.Y, G.Z + 1) == KNOWN) { Sum += VelField[VelIndex].Cell(G.X, G.Y, G.Z + 1); Count++; }

				TI_ASSERT(Count != 0);
				VelField[VelIndex].Cell(G) = Sum / (float)Count;
				Status[VelIndex].Cell(G) = KNOWN;
			}
		}
	}
}

void FFluidSolverFlipCPU::MarkCells()
{
	Marker.Clear();
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		const vector3df& PosInGrid = Particles.GetParticlePosition(Index);
		vector3di IndexInGrid = Floor(PosInGrid * InvCellSize);
		Marker.Cell(IndexInGrid) = Marker_Fluid;
	}
}

void FFluidSolverFlipCPU::BackupVelocity()
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[0].GetTotalCells(); Index++)
	{
		VelFieldDelta[0].Cell(Index) = VelField[0].Cell(Index);
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[1].GetTotalCells(); Index++)
	{
		VelFieldDelta[1].Cell(Index) = VelField[1].Cell(Index);
	}
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		VelFieldDelta[2].Cell(Index) = VelField[2].Cell(Index);
	}
}

void FFluidSolverFlipCPU::CalcExternalForces(float Dt)
{
	const vector3df Gravity = vector3df(0.f, 0.f, -9.8f);
	const vector3df DV = Gravity * Dt;
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < VelField[2].GetTotalCells(); Index++)
	{
		vector3di GridIndex = VelField[2].ArrayIndexToGridIndex(Index);
		if (Marker.IsWMarkerValueEqual(Marker_Fluid, GridIndex.X, GridIndex.Y, GridIndex.Z))
		{
			VelField[2].Cell(Index) += DV.Z;
		}
	}
}

void FFluidSolverFlipCPU::CalcVisicosity(float Dt)
{

}

void FFluidSolverFlipCPU::SolvePressure(float Dt)
{
	Pressure.Clear();

	PCGSolverParameters Parameter;
	Parameter.CellSize = CellSize;
	Parameter.DeltaTime = Dt;
	Parameter.U = &VelField[0];
	Parameter.V = &VelField[1];
	Parameter.W = &VelField[2];
	Parameter.Marker = &Marker;
	Parameter.Pressure = &Pressure;

	PCGSolver.Solve(Parameter);
}

void FFluidSolverFlipCPU::ApplyPressure(float Dt)
{
	TI_ASSERT(Pressure.GetTotalCells() == Marker.GetTotalCells());
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Pressure.GetTotalCells(); Index++)
	{
		vector3di GridIndex = Pressure.ArrayIndexToGridIndex(Index);

		if (Marker.Cell(Index) == Marker_Fluid)
		{
			float P = Pressure.Cell(GridIndex);

			if (GridIndex.X > 0)
			{
				float UP0 = Pressure.Cell(GridIndex.X - 1, GridIndex.Y, GridIndex.Z);
				VelField[0].Cell(Index) -= (P - UP0) * Dt * InvCellSize.X;
			}

			if (GridIndex.Y > 0)
			{
				float VP0 = Pressure.Cell(GridIndex.X, GridIndex.Y - 1, GridIndex.Z);
				VelField[1].Cell(Index) -= (P - VP0) * Dt * InvCellSize.Y;
			}

			if (GridIndex.Z > 0)
			{
				float WP0 = Pressure.Cell(GridIndex.X, GridIndex.Y, GridIndex.Z - 1);
				VelField[2].Cell(Index) -= (P - WP0) * Dt * InvCellSize.Z;
			}
		}
	}
}

void FFluidSolverFlipCPU::ConstrainVelocityField()
{
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Marker.GetTotalCells(); Index++)
	{
		vector3di GridIndex = Marker.ArrayIndexToGridIndex(Index);
		if (Marker.Cell(Index) == Marker_Air)
		{
			VelField[0].Cell(GridIndex) = 0.f;
			VelField[1].Cell(GridIndex) = 0.f;
			VelField[2].Cell(GridIndex) = 0.f;
		}

	}
}

void FFluidSolverFlipCPU::AdvectParticles(float Dt)
{
	vector3df CellSizeH = CellSize * 0.5f;
	vector3df VelocityOffset[3] =
	{
		vector3df(0.f, CellSizeH.Y, CellSizeH.Z),
		vector3df(CellSizeH.X, 0.f, CellSizeH.Z),
		vector3df(CellSizeH.X, CellSizeH.Y, 0.f)
	};
#if DO_PARALLEL
#pragma omp parallel for
#endif
	for (int32 Index = 0; Index < Particles.GetTotalParticles(); Index++)
	{
		// Update Particle Velocity
		const vector3df& P = Particles.GetParticlePosition(Index);
		vector3df RelativePU = MaxVector3d((P - VelocityOffset[0]) * InvCellSize, vector3df());
		vector3df RelativePV = MaxVector3d((P - VelocityOffset[1]) * InvCellSize, vector3df());
		vector3df RelativePW = MaxVector3d((P - VelocityOffset[2]) * InvCellSize, vector3df());

		float U0 = VelField[0].SampleByRelativePositionLinear(RelativePU);
		float V0 = VelField[1].SampleByRelativePositionLinear(RelativePV);
		float W0 = VelField[2].SampleByRelativePositionLinear(RelativePW);

		vector3df NewV = vector3df(U0, V0, W0);
		Particles.SetParticleVelocity(Index, NewV);

		// Advect particles with RK2
		vector3df SamplePosition = P + NewV * Dt * 0.5f;
		vector3df SamplePU = MaxVector3d((SamplePosition - VelocityOffset[0]) * InvCellSize, vector3df());
		vector3df SamplePV = MaxVector3d((SamplePosition - VelocityOffset[1]) * InvCellSize, vector3df());
		vector3df SamplePW = MaxVector3d((SamplePosition - VelocityOffset[2]) * InvCellSize, vector3df());

		float U1 = VelField[0].SampleByRelativePositionLinear(SamplePU);
		float V1 = VelField[1].SampleByRelativePositionLinear(SamplePV);
		float W1 = VelField[2].SampleByRelativePositionLinear(SamplePW);
		vector3df SampledVel = vector3df(U1, V1, W1);
		vector3df NewP = P + SampledVel * Dt;
		Particles.SetParticlePosition(Index, NewP);
	}
}