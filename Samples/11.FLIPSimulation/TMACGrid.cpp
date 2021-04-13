/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TMACGrid.h"
#include "FLIPSimRenderer.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
using namespace rapidjson;

TMACGrid::TMACGrid()
	: Seperation(1.f)
{
}

TMACGrid::~TMACGrid()
{
}

void DebugMarkers(const TArray3<int32>& Markers)
{
	vector3di Size = Markers.GetSize();

	TVector<vector3df> Positions;
	TVector<vector3df> Velocities;
	Positions.reserve(Size.X * Size.Y * Size.Z);
	Velocities.reserve(Size.X * Size.Y * Size.Z);

	const float Sep = 0.5f;
	const float SepH = Sep * 0.5f;
	for (int32 z = 0; z < Size.Z; z++)
	{
		for (int32 y = 0; y < Size.Y; y++)
		{
			for (int32 x = 0; x < Size.X; x++)
			{
				Positions.push_back(vector3df(x * Sep + SepH, y * Sep + SepH, z * Sep + SepH));
				int32 Index = Markers.GetAccessIndex(x, y, z);
				Velocities.push_back(vector3df(Markers[Index], 0, 0));
			}
		}
	}

	Document JsonDoc;
	Value ArrayPosition(kArrayType);
	Value ArrayVelocity(kArrayType);
	Document::AllocatorType& Allocator = JsonDoc.GetAllocator();

	JsonDoc.SetObject();
	ArrayPosition.Reserve((uint32)Positions.size() * 3, Allocator);
	ArrayVelocity.Reserve((uint32)Positions.size() * 3, Allocator);
	for (uint32 i = 0; i < Positions.size(); i++)
	{
		ArrayPosition.PushBack(Positions[i].X, Allocator);
		ArrayPosition.PushBack(Positions[i].Y, Allocator);
		ArrayPosition.PushBack(Positions[i].Z, Allocator);

		ArrayVelocity.PushBack(Velocities[i].X, Allocator);
		ArrayVelocity.PushBack(Velocities[i].Y, Allocator);
		ArrayVelocity.PushBack(Velocities[i].Z, Allocator);
	}

	Value TotalParticles(kNumberType);
	TotalParticles.SetInt((int32)Positions.size());
	JsonDoc.AddMember("count", TotalParticles, Allocator);
	JsonDoc.AddMember("positions", ArrayPosition, Allocator);
	JsonDoc.AddMember("velocities", ArrayVelocity, Allocator);

	StringBuffer StrBuffer;
	Writer<StringBuffer> JsonWriter(StrBuffer);
	JsonDoc.Accept(JsonWriter);

	TFile Output;
	if (Output.Open("DebugGrid.json", EFA_CREATEWRITE))
	{
		Output.Write(StrBuffer.GetString(), (int32)StrBuffer.GetSize());
		Output.Close();
	}
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
				if ((x == 0 || x == Size.X - 1) ||
					(y == 0 || y == Size.Y - 1) ||
					(z == 0 || z == Size.Z - 1))
				{
					int32 Index = GetAccessIndex(vector3di(x, y, z));
					Markers[Index] = GridSolid;
				}
			}
		}
	}

	//DebugMarkers(Markers);
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