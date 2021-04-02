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
	void GetAdjacentGrid(const vector3df& InPos, TVector<vector3di>& OutputIndices, TVector<float>& OutputWeights);
	void ClearVelocities();

	int32 GetAccessIndex(const vector3di& Index)
	{
		return Index.Z * (Size.X * Size.Y) + Index.Y * Size.X + Index.X;
	}


protected:
	vector3di Size;
	float Seperation;

	TVector<float> U, V, W;

	friend class TFlipSolver;
};