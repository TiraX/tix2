/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TMACGrid
{
public:
	TMACGrid();
	~TMACGrid();

	void InitSize(const vector3di& InSize, float InSeperation);
	void GetAdjacentGrid(const vectype& InPos, TVector<vector3di>& OutputIndices, TVector<ftype>& OutputWeights);
	void ClearGrids();

	ftype DivergenceAtCellCenter(int32 x, int32 y, int32 z);

	int32 GetAccessIndex(const vector3di& Index)
	{
		return Index.Z * (Size.X * Size.Y) + Index.Y * Size.X + Index.X;
	}
	int32 GetAccessIndex(int32 x, int32 y, int32 z)
	{
		return z * (Size.X * Size.Y) + y * Size.X + x;
	}

	enum {
		GridAir,
		GridFluid,
		GridSolid,
	};
protected:

protected:
	vector3di Size;
	float Seperation;

	TArray3<ftype> U, V, W;
	TArray3<int32> Markers;

	friend class TFlipSolver;
};