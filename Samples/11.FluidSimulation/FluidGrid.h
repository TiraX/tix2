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