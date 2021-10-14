/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "FluidSolver.h"

enum GridBoundaryType
{
	Boundary_None = 0,
	Boundary_Left = 1 << 0,
	Boundary_Right = 1 << 1,
	Boundary_Front = 1 << 2,
	Boundary_Back = 1 << 3,
	Boundary_Up = 1 << 4,
	Boundary_Bottom = 1 << 5
};

template<class T>
class FFluidGrid3
{
public:

	FFluidGrid3()
	{
	}
	FFluidGrid3(const vector3di& Dim)
	{
		Create(Dim);
	}

	void Create(const vector3di& Dim)
	{
		Dimension = Dim;

		Cells.clear();
		Cells.resize(Dim.X * Dim.Y * Dim.Z);
	}

	void Clear()
	{
		memset(Cells.data(), 0, Cells.size() * sizeof(T));
	}

	const vector3di& GetDimension() const
	{
		return Dimension;
	}

	int32 GetTotalCells() const
	{
		return (int32)Cells.size();
	}

	uint32 GetBoudaryTypeInfo(int32 Index) const
	{
		uint32 Result = 0;
		vector3di CellIndex = ArrayIndexToGridIndex(Index);

		if (CellIndex.X == 0)
			Result |= Boundary_Left;
		else if (CellIndex.X == (Dimension.X - 1))
			Result |= Boundary_Right;

		if (CellIndex.Y == 0)
			Result |= Boundary_Front;
		else if (CellIndex.Y == (Dimension.Y - 1))
			Result |= Boundary_Back;

		if (CellIndex.Z == 0)
			Result |= Boundary_Up;
		else if (CellIndex.Z == (Dimension.Z - 1))
			Result |= Boundary_Bottom;

		return Result;
	}

	T& Cell(const vector3di& Index)
	{
		return Cell(Index.X, Index.Y, Index.Z);
	}
	const T& Cell(const vector3di& Index) const
	{
		return Cell(Index.X, Index.Y, Index.Z);
	}
	T& Cell(int32 i, int32 j, int32 k)
	{
		return Cells[GridIndexToArrayIndex(i, j, k)];
	}
	const T& Cell(int32 i, int32 j, int32 k) const
	{
		return Cells[GridIndexToArrayIndex(i, j, k)];
	}
	T& Cell(int32 Index)
	{
		return Cells[Index];
	}
	const T& Cell(int32 Index) const
	{
		return Cells[Index];
	}
	int32 GridIndexToArrayIndex(int32 i, int32 j, int32 k) const
	{
		ValidateGridIndex(i, j, k);
		int32 Index = k * Dimension.X * Dimension.Y + j * Dimension.X + i;
		return Index;
	}

	const T& SafeCell(const vector3di& Index) const
	{
		return SafeCell(Index.X, Index.Y, Index.Z);
	}
	const T& SafeCell(int32 i, int32 j, int32 k) const
	{
		return Cells[GridIndexToArrayIndexSafe(i, j, k)];
	}
	int32 GridIndexToArrayIndexSafe(int32 i, int32 j, int32 k) const
	{
		i = TMath::Clamp(i, 0, Dimension.X - 1);
		j = TMath::Clamp(j, 0, Dimension.Y - 1);
		k = TMath::Clamp(k, 0, Dimension.Z - 1);
		int32 Index = k * Dimension.X * Dimension.Y + j * Dimension.X + i;
		return Index;
	}
	vector3di ArrayIndexToGridIndex(int32 Index) const
	{
		vector3di Result;
		Result.Z = Index / (Dimension.X * Dimension.Y);
		Result.Y = (Index % (Dimension.X * Dimension.Y)) / Dimension.X;
		Result.X = Index % Dimension.X;
		return Result;
	}

	// Relative Position means (Position - Origin) * InvCellSize
	T SampleByRelativePositionLinear(const vector3df& P)
	{
		vector3di GridIndex;
		GridIndex.X = TMath::Floor(P.X);
		GridIndex.Y = TMath::Floor(P.Y);
		GridIndex.Z = TMath::Floor(P.Z);
		ValidateGridIndex(GridIndex.X, GridIndex.Y, GridIndex.Z);

		float iX = TMath::Frac(P.X);
		float iY = TMath::Frac(P.Y);
		float iZ = TMath::Frac(P.Z);
		float iX1 = 1.f - iX;
		float iY1 = 1.f - iY;
		float iZ1 = 1.f - iZ;

		T Values[8] = { 0 };
		bool ValidU1 = GridIndex.X + 1 < Dimension.X;
		bool ValidV1 = GridIndex.Y + 1 < Dimension.Y;
		bool ValidW1 = GridIndex.Z + 1 < Dimension.Z;

		Values[0] = Cell(GridIndex);
		if (ValidU1)
			Values[1] = Cell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z);
		if (ValidV1)
			Values[2] = Cell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z);
		if (ValidW1)
			Values[3] = Cell(GridIndex.X, GridIndex.Y, GridIndex.Z + 1);
		if (ValidU1 && ValidW1)
			Values[4] = Cell(GridIndex.X + 1, GridIndex.Y, GridIndex.Z + 1);
		if (ValidV1 && ValidW1)
			Values[5] = Cell(GridIndex.X, GridIndex.Y + 1, GridIndex.Z + 1);
		if (ValidU1 && ValidV1)
			Values[6] = Cell(GridIndex.X + 1, GridIndex.Y + 1, GridIndex.Z);
		if (ValidU1 && ValidV1 && ValidW1)
			Values[7] = Cell(GridIndex.X + 1, GridIndex.Y + 1, GridIndex.Z + 1);

		return Values[0] * iX1 * iY1 * iZ1 +
			Values[1] * iX * iY1 * iZ1 +
			Values[2] * iX1 * iY * iZ1 +
			Values[3] * iX1 * iY1 * iZ +
			Values[4] * iX * iY1 * iZ +
			Values[5] * iX1 * iY * iZ +
			Values[6] * iX * iY * iZ1 +
			Values[7] * iX * iY * iZ;
	}
private:
	void ValidateGridIndex(int32 i, int32 j, int32 k) const
	{
		TI_ASSERT(i >= 0 && i < Dimension.X);
		TI_ASSERT(j >= 0 && j < Dimension.Y);
		TI_ASSERT(k >= 0 && k < Dimension.Z);
	}
private:
	vector3di Dimension;
	TVector<T> Cells;
};