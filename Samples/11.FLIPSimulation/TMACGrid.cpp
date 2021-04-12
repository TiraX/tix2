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

	U.Resize(Size);
	V.Resize(Size);
	W.Resize(Size);

	// Init markers and set solid mark
	Markers.Resize(Size);
	for (int32 z = 0; z < Size.Z; z++)
	{
		for (int32 y = 0; y < Size.Y; y++)
		{
			for (int32 x = 0; x < Size.X; x++)
			{
				vector3di D = Size - vector3di(x, y, z);
				if (TMath::Abs(D.X) == 0 || TMath::Abs(D.Y) == 0 || TMath::Abs(D.Z) == 0)
				{
					int32 Index = GetAccessIndex(vector3di(x, y, z));
					Markers[Index] = GridSolid;
				}
			}
		}
	}
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

void TMACGrid::ClearGrids()
{
	// Clear velocities
	U.ResetZero();
	V.ResetZero();
	W.ResetZero();

	// Clear grid markers
	for (int32 z = 0; z < Size.Z; z++)
	{
		for (int32 y = 0; y < Size.Y; y++)
		{
			for (int32 x = 0; x < Size.X; x++)
			{
				int32 Index = GetAccessIndex(vector3di(x, y, z));
				if (Markers[Index] != GridSolid)
				{
					Markers[Index] = GridAir;
				}
			}
		}
	}
}

float TMACGrid::DivergenceAtCellCenter(int32 x, int32 y, int32 z)
{
	int32 Index = GetAccessIndex(x, y, z);
	int32 IndexX1 = GetAccessIndex(x + 1, y, z);
	int32 IndexY1 = GetAccessIndex(x, y + 1, z);
	int32 IndexZ1 = GetAccessIndex(x, y, z + 1);

	float U0 = U[Index];
	float U1 = U[IndexX1];
	float V0 = V[Index];
	float V1 = V[IndexY1];
	float Z0 = W[Index];
	float Z1 = W[IndexZ1];

	return (U1 - U0) / Seperation + (V1 - V0) / Seperation + (Z1 - Z0) / Seperation;
}