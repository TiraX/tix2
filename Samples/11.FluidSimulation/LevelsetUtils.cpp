/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "LevelsetUtils.h"

//Given two signed distance values (line endpoints), determine what fraction of a connecting segment is "inside"
float FractionInside(float SdfLeft, float SdfRight)
{
	if (SdfLeft < 0 && SdfRight < 0)
	{
		return 1.f;
	}
	if (SdfLeft < 0 && SdfRight >= 0)
	{
		return SdfLeft / (SdfLeft - SdfRight);
	}
	if (SdfLeft >= 0 && SdfRight < 0)
	{
		return SdfRight / (SdfRight - SdfLeft);
	}

	return 0.f;
}
void CycleArray(float* Arr, int Size)
{
	float Temp = Arr[0];
	for (int i = 0; i < Size - 1; ++i)
	{
		Arr[i] = Arr[i + 1];
	}
	Arr[Size - 1] = Temp;
}
//Given four signed distance values (square corners), determine what fraction of the square is "inside"
float FractionInside(float SdfBL, float SdfBR, float SdfTL, float SdfTR)
{

	int32 insideCount =
		(SdfBL < 0 ? 1 : 0) +
		(SdfTL < 0 ? 1 : 0) +
		(SdfBR < 0 ? 1 : 0) +
		(SdfTR < 0 ? 1 : 0);
	float list[] = { SdfBL, SdfBR, SdfTR, SdfTL };

	if (insideCount == 4)
	{
		return 1.f;
	}
	else if (insideCount == 3)
	{
		//rotate until the positive value is in the first position
		while (list[0] < 0)
		{
			CycleArray(list, 4);
		}

		//Work out the area of the exterior triangle
		float side0 = 1 - FractionInside(list[0], list[3]);
		float side1 = 1 - FractionInside(list[0], list[1]);
		return 1.0f - 0.5f * side0 * side1;
	}
	else if (insideCount == 2) {

		//rotate until a negative value is in the first position, and the next negative is in either slot 1 or 2.
		while (list[0] >= 0 || !(list[1] < 0 || list[2] < 0))
		{
			CycleArray(list, 4);
		}

		if (list[1] < 0)
		{
			//the matching signs are adjacent
			float sideLeft = FractionInside(list[0], list[3]);
			float sideRight = FractionInside(list[1], list[2]);
			return  0.5f * (sideLeft + sideRight);
		}
		else
		{
			//matching signs are diagonally opposite
			//determine the centre point's sign to disambiguate this case
			float middlePoint = 0.25f * (list[0] + list[1] + list[2] + list[3]);
			if (middlePoint < 0)
			{
				float area = 0;

				//first triangle (top left)
				float side1 = 1 - FractionInside(list[0], list[3]);
				float side3 = 1 - FractionInside(list[2], list[3]);

				area += 0.5f * side1 * side3;

				//second triangle (top right)
				float side2 = 1 - FractionInside(list[2], list[1]);
				float side0 = 1 - FractionInside(list[0], list[1]);
				area += 0.5f * side0 * side2;

				return 1.0f - area;
			}
			else
			{
				float area = 0;

				//first triangle (bottom left)
				float side0 = FractionInside(list[0], list[1]);
				float side1 = FractionInside(list[0], list[3]);
				area += 0.5f * side0 * side1;

				//second triangle (top right)
				float side2 = FractionInside(list[2], list[1]);
				float side3 = FractionInside(list[2], list[3]);
				area += 0.5f * side2 * side3;
				return area;
			}
		}
	}
	else if (insideCount == 1)
	{
		//rotate until the negative value is in the first position
		while (list[0] >= 0)
		{
			CycleArray(list, 4);
		}

		//Work out the area of the interior triangle, and subtract from 1.
		float side0 = FractionInside(list[0], list[3]);
		float side1 = FractionInside(list[0], list[1]);
		return 0.5f * side0 * side1;
	}
	else
	{
		return 0.f;
	}
}

float DistanceAtCellCenter(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k)
{
	return 0.125f * (
		SDF.Cell(i, j, k) +
		SDF.Cell(i + 1, j, k) +
		SDF.Cell(i, j + 1, k) +
		SDF.Cell(i + 1, j + 1, k) +
		SDF.Cell(i, j, k + 1) +
		SDF.Cell(i + 1, j, k + 1) +
		SDF.Cell(i, j + 1, k + 1) +
		SDF.Cell(i + 1, j + 1, k + 1)
		);
}
float DistanceAtCellCenter(FGeometrySDF* SDF, const vector3df& Pos)
{
	return SDF->SampleSDFByPosition(Pos);
}