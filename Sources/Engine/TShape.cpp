/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TShape.h"

namespace tix
{
	void _CreateICOSphere(uint32 Frequency, TVector<vector3df>& OutPositions, TVector<uint16>& OutIndices)
	{
		TI_ASSERT(Frequency >= 1 && Frequency < 80);

		// Reserve space for containers
		static const uint32 PointsByFrequency[] = { 12, 42, 92, 162, 252, 362, 492, 642, 812, 1002};
		static const uint32 FacesByFrequency[] = { 20, 80, 180, 320, 500, 720, 980, 1280, 1620, 2000};
		uint32 FreqIndex = ti_min(Frequency - 1, 9);
		OutPositions.reserve(PointsByFrequency[FreqIndex]);
		OutIndices.reserve(FacesByFrequency[FreqIndex] * 3);

		const float T = (1.f + 2.236f) / 2.f;
		const vector3df BasePos[] =
		{
			vector3df(-1.f, T, 0.f).normalize(),
			vector3df(1.f, T, 0.f).normalize(),
			vector3df(-1.f, -T, 0.f).normalize(),
			vector3df(1.f, -T, 0.f).normalize(),

			vector3df(0.f, -1.f, T).normalize(),
			vector3df(0.f, 1.f, T).normalize(),
			vector3df(0.f, -1.f, -T).normalize(),
			vector3df(0.f, 1.f, -T).normalize(),

			vector3df(T, 0.f, -1.f).normalize(),
			vector3df(T, 0.f, 1.f).normalize(),
			vector3df(-T, 0.f, -1.f).normalize(),
			vector3df(-T, 0.f, 1.f).normalize()
		};

		const vector3di BaseFaces[] =
		{
			vector3di(0, 11, 5),
			vector3di(0, 5, 1),
			vector3di(0, 1, 7),
			vector3di(0, 7, 10),
			vector3di(0, 10, 11),

			vector3di(1, 5, 9),
			vector3di(5, 11, 4),
			vector3di(11, 10, 2),
			vector3di(10, 7, 6),
			vector3di(7, 1, 8),

			vector3di(3, 9, 4),
			vector3di(3, 4, 2),
			vector3di(3, 2, 6),
			vector3di(3, 6, 8),
			vector3di(3, 8, 9),

			vector3di(4, 9, 5),
			vector3di(2, 4, 11),
			vector3di(6, 2, 10),
			vector3di(8, 6, 7),
			vector3di(9, 8, 1)
		};

		const uint32 Faces = sizeof(BaseFaces) / sizeof(vector3di);
		const float FreqInv = 1.f / Frequency;
		// Key.X = StartPoint, Key.Y = EndPoint, Key.Z = Step
		THMap<vector3di, uint32> PointsMap;
		TVector<uint32> IndicesMap;

		for (uint32 f = 0; f < Faces; ++f)
		{
			const vector3di Face = BaseFaces[f];
			uint32 PointsOffset = uint32(OutPositions.size());
			//   p0
			//  /  \
		    // p1---p2
			const vector3df& P0 = BasePos[Face.X];
			const vector3df& P1 = BasePos[Face.Y];
			const vector3df& P2 = BasePos[Face.Z];

			auto MakeKey = [](int32 InStart, int32 InEnd, int32 Step, int32 Len)
			{
				bool bCorrectOrder = InStart < InEnd;
				vector3di Result;
				Result.X = bCorrectOrder ? InStart : InEnd;
				Result.Y = bCorrectOrder ? InEnd : InStart;
				Result.Z = bCorrectOrder ? Step : (Len - Step);
				return Result;
			};
			vector3di Key = MakeKey(Face.X, Face.X, 0, 0);

			vector3df StepLeft = (P1 - P0) * FreqInv;
			vector3df StepRight = (P2 - P0) * FreqInv;

			// Generate points
			auto AddSharedPoint = [](const vector3di& Key, const vector3df& Pos, THMap<vector3di, uint32>& PointsMap, TVector<vector3df>& OutPositions)
			{
				// Return shared point index
				if (PointsMap.find(Key) == PointsMap.end())
				{
					OutPositions.push_back(Pos);
					PointsMap[Key] = (uint32)(OutPositions.size() - 1);
				}
				TI_ASSERT(PointsMap[Key] < 65535);
				return uint16(PointsMap[Key]);
			};
			uint16 PointIndex = AddSharedPoint(Key, P0, PointsMap, OutPositions);
			IndicesMap.push_back(PointIndex);

			for (uint32 StepY = 0; StepY < Frequency; ++StepY)
			{
				vector3df PointLeft = (P0 + StepLeft * float(StepY + 1)).normalize();
				vector3df PointRight = (P0 + StepRight * float(StepY + 1)).normalize();

				float FreqHorizonInv = 1.f / (StepY + 1);
				vector3df StepHorizon = (PointRight - PointLeft) * FreqHorizonInv;

				vector3di KeyLeft = MakeKey(Face.X, Face.Y, StepY, Frequency);
				vector3di KeyRight = MakeKey(Face.X, Face.Z, StepY, Frequency);
				PointIndex = AddSharedPoint(KeyLeft, PointLeft, PointsMap, OutPositions);
				IndicesMap.push_back(PointIndex);
				for (uint32 StepX = 0; StepX < StepY; ++StepX)
				{
					// Interpolated point will never be shared, add to OutPositions directly
					vector3df P = (PointLeft + StepHorizon * float(StepX + 1)).normalize();
					OutPositions.push_back(P);
					PointIndex = uint32(OutPositions.size() - 1);
					IndicesMap.push_back(PointIndex);
				}
				PointIndex = AddSharedPoint(KeyRight, PointRight, PointsMap, OutPositions);
				IndicesMap.push_back(PointIndex);
			}

			// Generate indices
			int32 Index = PointsOffset;
			for (uint32 StepY = 0; StepY < Frequency; ++StepY)
			{
				uint32 PointCount = StepY + 1;

				for (uint32 StepX = 0; StepX < PointCount; ++StepX)
				{
					vector3di NewFace;
					NewFace.X = Index + StepX;	// Current Point
					NewFace.Y = Index + StepX + PointCount;	// Next Row Left
					NewFace.Z = Index + StepX + PointCount + 1;	// Next Row Right
					OutIndices.push_back(IndicesMap[NewFace.X]);
					OutIndices.push_back(IndicesMap[NewFace.Y]);
					OutIndices.push_back(IndicesMap[NewFace.Z]);

					if (StepX != PointCount - 1)
					{
						NewFace.X = Index + StepX;
						NewFace.Y = Index + StepX + PointCount + 1;
						NewFace.Z = Index + StepX + 1;
						OutIndices.push_back(IndicesMap[NewFace.X]);
						OutIndices.push_back(IndicesMap[NewFace.Y]);
						OutIndices.push_back(IndicesMap[NewFace.Z]);
					}
				}
				Index += PointCount;
			}
		}
	}

	void _CreateLongLatitudeSphere(uint32 Longitude, uint32 Latitude, TVector<vector3df>& OutPositions, TVector<uint16>& OutIndices)
	{
		//(x, y, z) = (sin(Pi * m/M) cos(2Pi * n/N), sin(Pi * m/M) sin(2Pi * n/N), cos(Pi * m/M))
		OutPositions.clear();
		OutIndices.clear();
		OutPositions.reserve(Longitude * (Latitude - 1) + 2);
		OutIndices.reserve(Longitude * (Latitude - 2) * 6 + Longitude * 2 * 3);

		// Calculate positions
		OutPositions.push_back(vector3df(0, 0, 1.f));
		for (uint32 Lat = 1 ; Lat < Latitude; ++ Lat)
		{
			for (uint32 Long = 0 ; Long < Longitude ; ++ Long)
			{
				vector3df Pos;
				Pos.X = sin(PI * Lat / Latitude) * cos(PI * 2.f * Long / Longitude);
				Pos.Y = sin(PI * Lat / Latitude) * sin(PI * 2.f * Long / Longitude);
				Pos.Z = cos(PI * Lat / Latitude);
				OutPositions.push_back(Pos);
			}
		}
		OutPositions.push_back(vector3df(0, 0, -1.f));

		// Create indices
		// First Latitude
		for (uint32 Tri = 0 ; Tri < Longitude ; ++ Tri)
		{
			OutIndices.push_back(0);
			OutIndices.push_back((Tri + 1) % Longitude + 1);
			OutIndices.push_back(Tri + 1);
		}

		// Middle faces
		for (uint32 i = 0; i < Latitude - 2; ++i)
		{
			int CurrLineStart = Longitude * i + 1;
			int NextLineStart = CurrLineStart + Longitude;

			for (uint32 l = 0; l < Longitude; ++l) {
				int curr0, curr1, next0, next1;
				curr0 = l + CurrLineStart;
				curr1 = (l + 1) % Longitude + CurrLineStart;
				next0 = l + NextLineStart;
				next1 = (l + 1) % Longitude + NextLineStart;
				OutIndices.push_back(curr0);
				OutIndices.push_back(curr1);
				OutIndices.push_back(next0);

				OutIndices.push_back(next0);
				OutIndices.push_back(curr1);
				OutIndices.push_back(next1);
			}
			CurrLineStart += Longitude;
		}

		// Last Latitude
		int32 LastPoint = (int32)OutPositions.size() - 1;
		int32 LastLatStart = (Latitude - 2) * Longitude;
		for (uint32 Tri = 0; Tri < Longitude; ++Tri)
		{
			OutIndices.push_back(LastLatStart + Tri + 1);
			OutIndices.push_back(LastLatStart + (Tri + 1) % Longitude + 1);
			OutIndices.push_back(LastPoint);
		}
	}

	void _CreateUnitBox(TVector<vector3df>& OutPositions, TVector<uint16>& OutIndices)
	{
		const vector3df Points[] =
		{ 
			vector3df(-1.f, -1.f, -1.f),
			vector3df( 1, -1.f, -1.f),
			vector3df( 1.f, -1.f, 1),
			vector3df(-1.f, -1.f, 1),
			vector3df(-1.f, 1.f, -1.f),
			vector3df( 1.f, 1.f, -1.f),
			vector3df( 1.f, 1.f, 1.f),
			vector3df(-1.f, 1.f, 1.f)
		};
		const uint16 Faces[] =
		{
			1, 4, 5,
			2, 5, 6,

			3, 6, 7,
			0, 7, 4,

			2, 0, 1,
			5, 7, 6,
			
			7, 5, 4,
			0, 2, 3,
			
			7, 0, 3,
			6, 3, 2,
			
			5, 2, 1,
			4, 1, 0
		};
		OutPositions.resize(8);
		memcpy(OutPositions.data(), Points, sizeof(Points));

		OutIndices.resize(36);
		memcpy(OutIndices.data(), Faces, sizeof(Faces));
	}

	void TShape::CreateICOSphere(
		uint32 Frequency,
		const vector3df& Center, 
		float Radius, 
		TVector<vector3df>& OutPositions, 
		TVector<uint16>& OutIndices)
	{
		TVector<vector3df> SpherePositions;
		TVector<uint16> SphereIndices;
		_CreateICOSphere(Frequency, SpherePositions, SphereIndices);

		const uint16 IndexOffset = (uint16)(OutPositions.size());
		for (const auto& P : SpherePositions)
		{
			vector3df NewP = P * Radius + Center;
			OutPositions.push_back(NewP);
		}
		for (const auto& I : SphereIndices)
		{
			uint16 Index = I + IndexOffset;
			OutIndices.push_back(Index);
		}
	} 

	void TShape::CreateBox(
		const vector3df& Center,
		const vector3df& Edges,
		const quaternion& Rotation,
		TVector<vector3df>& OutPositions,
		TVector<uint16>& OutIndices)
	{
		TVector<vector3df> BoxPositions;
		TVector<uint16> BoxIndices;
		_CreateUnitBox(BoxPositions, BoxIndices);

		matrix4 RotMat;
		Rotation.getMatrix(RotMat);

		const uint16 IndexOffset = (uint16)(OutPositions.size());
		for (const auto& P : BoxPositions)
		{
			vector3df NewP;
			RotMat.transformVect(NewP, P * Edges);
			NewP += Center;
			OutPositions.push_back(NewP);
		}
		for (const auto& I : BoxIndices)
		{
			uint16 Index = I + IndexOffset;
			OutIndices.push_back(Index);
		}
	}

	void TShape::CreateCapsule(
		const vector3df& Center,
		float Radius,
		float Length,
		const quaternion& Rotation,
		TVector<vector3df>& OutPositions,
		TVector<uint16>& OutIndices)
	{
		TI_ASSERT(0);
	}
}