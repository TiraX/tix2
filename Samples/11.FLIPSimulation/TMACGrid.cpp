/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMACGrid.h"
#include "FLIPSimRenderer.h"

TMACGrid::TMACGrid()
	: Seperation(1.f)
{
}

TMACGrid::~TMACGrid()
{
}

void TMACGrid::InitSize(const vector3di& InSize, float InSeperation)
{
	Size = InSize;
	Seperation = InSeperation;

	int32 TotalCount = Size.X * Size.Y * Size.Z;
	U.resize(TotalCount);
	V.resize(TotalCount);
	W.resize(TotalCount);
}

void TMACGrid::GetAdjacentGrid(const vector3df& InPos, TVector<vector3di>& OutputIndices, TVector<float>& OutputWeights)
{
	OutputIndices.clear();
	OutputIndices.resize(8);
	OutputWeights.clear();
	OutputWeights.resize(8);

	vector3df Coord = InPos / Seperation;
	vector3df RCoord = Coord - vector3df(0.5f, 0.5f, 0.5f);
	uint32 X0 = uint32(floor(RCoord.X));
	uint32 Y0 = uint32(floor(RCoord.Y));
	uint32 Z0 = uint32(floor(RCoord.Z));
	uint32 X1 = TMath::Min(X0 + 1, uint32(Size.X - 1));
	uint32 Y1 = TMath::Min(Y0 + 1, uint32(Size.Y - 1));
	uint32 Z1 = TMath::Min(Z0 + 1, uint32(Size.Z - 1));

	OutputIndices[0] = vector3di(X0, Y0, Z0);
	OutputIndices[1] = vector3di(X1, Y0, Z0);
	OutputIndices[2] = vector3di(X0, Y1, Z0);
	OutputIndices[3] = vector3di(X1, Y1, Z0);
	OutputIndices[4] = vector3di(X0, Y0, Z1);
	OutputIndices[5] = vector3di(X1, Y0, Z1);
	OutputIndices[6] = vector3di(X0, Y1, Z1);
	OutputIndices[7] = vector3di(X1, Y1, Z1);

	vector3df Frac = RCoord - vector3df(floor(RCoord.X), floor(RCoord.Y), floor(RCoord.Z));
	OutputWeights[0] = (1.f - Frac.X) * (1.f - Frac.Y) * (1.f - Frac.Z);
	OutputWeights[1] = Frac.X * (1.f - Frac.Y) * (1.f - Frac.Z);
	OutputWeights[2] = (1.f - Frac.X) * Frac.Y * (1.f - Frac.Z);
	OutputWeights[3] = Frac.X * Frac.Y * (1.f - Frac.Z);
	OutputWeights[4] = (1.f - Frac.X) * (1.f - Frac.Y) * Frac.Z;
	OutputWeights[5] = Frac.X * (1.f - Frac.Y) * Frac.Z;
	OutputWeights[6] = (1.f - Frac.X) * Frac.Y * Frac.Z;
	OutputWeights[7] = Frac.X * Frac.Y * Frac.Z;
}

void TMACGrid::ClearVelocities()
{
	memset(U.data(), 0, U.size() * sizeof(float));
	memset(V.data(), 0, V.size() * sizeof(float));
	memset(W.data(), 0, W.size() * sizeof(float));
}