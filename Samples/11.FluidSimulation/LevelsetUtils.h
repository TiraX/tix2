/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "GeometrySDF.h"
#include "FluidGrid.h"

//Given two signed distance values (line endpoints), determine what fraction of a connecting segment is "inside"
float FractionInside(float SdfLeft, float SdfRight);
//Given four signed distance values (square corners), determine what fraction of the square is "inside"
float FractionInside(float SdfBL, float SdfBR, float SdfTL, float SdfTR);
float DistanceAtCellCenter(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k);


//inline float GetFaceWeightU(FGeometrySDF* SDF, const vector3df& P, const vector3df& CellSize)
//{
//	float Sdf0 = SDF->SampleSDFByPosition(P);
//	float Sdf1 = SDF->SampleSDFByPosition(P + vector3df(0.f, CellSize.Y, 0.f));
//	float Sdf2 = SDF->SampleSDFByPosition(P + vector3df(0.f, 0.f, CellSize.Z));
//	float Sdf3 = SDF->SampleSDFByPosition(P + vector3df(0.f, CellSize.Y, CellSize.Z));
//	return FractionInside(Sdf0, Sdf1, Sdf2, Sdf3);
//}
//inline float GetFaceWeightV(FGeometrySDF* SDF, const vector3df& P, const vector3df& CellSize)
//{
//	float Sdf0 = SDF->SampleSDFByPosition(P);
//	float Sdf1 = SDF->SampleSDFByPosition(P + vector3df(0.f, 0.f, CellSize.Z));
//	float Sdf2 = SDF->SampleSDFByPosition(P + vector3df(CellSize.X, 0.f, 0.f));
//	float Sdf3 = SDF->SampleSDFByPosition(P + vector3df(CellSize.X, 0.f, CellSize.Z));
//	return FractionInside(Sdf0, Sdf1, Sdf2, Sdf3);
//}
//inline float GetFaceWeightW(FGeometrySDF* SDF, const vector3df& P, const vector3df& CellSize)
//{
//	float Sdf0 = SDF->SampleSDFByPosition(P);
//	float Sdf1 = SDF->SampleSDFByPosition(P + vector3df(0.f, CellSize.Y, 0.f));
//	float Sdf2 = SDF->SampleSDFByPosition(P + vector3df(CellSize.X, 0.f, 0.f));
//	float Sdf3 = SDF->SampleSDFByPosition(P + vector3df(CellSize.X, CellSize.Y, 0.f));
//	return FractionInside(Sdf0, Sdf1, Sdf2, Sdf3);
//}
inline float GetFaceWeight4U(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k)
{
	float Sdf0 = SDF.Cell(i, j, k);
	float Sdf1 = SDF.Cell(i, j + 1, k);
	float Sdf2 = SDF.Cell(i, j, k + 1);
	float Sdf3 = SDF.Cell(i, j + 1, k + 1);
	return FractionInside(Sdf0, Sdf1, Sdf2, Sdf3);
}
inline float GetFaceWeight4V(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k)
{
	float Sdf0 = SDF.Cell(i, j, k);
	float Sdf1 = SDF.Cell(i, j, k + 1);
	float Sdf2 = SDF.Cell(i + 1, j, k);
	float Sdf3 = SDF.Cell(i + 1, j, k + 1);
	return FractionInside(Sdf0, Sdf1, Sdf2, Sdf3);
}
inline float GetFaceWeight4W(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k)
{
	float Sdf0 = SDF.Cell(i, j, k);
	float Sdf1 = SDF.Cell(i, j + 1, k);
	float Sdf2 = SDF.Cell(i + 1, j, k);
	float Sdf3 = SDF.Cell(i + 1, j + 1, k);
	return FractionInside(Sdf0, Sdf1, Sdf2, Sdf3);
}
inline float GetFaceWeight2U(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k)
{
	float Sdf0 = SDF.Cell(i - 1, j, k);
	float Sdf1 = SDF.Cell(i, j, k);
	return FractionInside(Sdf0, Sdf1);
}
inline float GetFaceWeight2V(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k)
{
	float Sdf0 = SDF.Cell(i, j - 1, k);
	float Sdf1 = SDF.Cell(i, j, k);
	return FractionInside(Sdf0, Sdf1);
}
inline float GetFaceWeight2W(const FFluidGrid3<float>& SDF, int32 i, int32 j, int32 k)
{
	float Sdf0 = SDF.Cell(i, j, k - 1);
	float Sdf1 = SDF.Cell(i, j, k);
	return FractionInside(Sdf0, Sdf1);
}