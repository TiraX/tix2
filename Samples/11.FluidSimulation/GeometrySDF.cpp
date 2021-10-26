/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "GeometrySDF.h"

FBoxSDF* FGeometrySDF::CreateBoxSDF(const vector3df& MinPoint, const vector3df& MaxPoint, bool Invert)
{
	FBoxSDF* BoxSDF = ti_new FBoxSDF(MinPoint, MaxPoint);
	if (Invert)
		BoxSDF->InvertSDF();
	return BoxSDF;
}

FSphereSDF* FGeometrySDF::CreateSphereSDF(const vector3df& Center, float Radius, bool Invert)
{
	TI_ASSERT(0);
	return nullptr;
}

//////////////////////////////////////////////////

FBoxSDF::FBoxSDF(const vector3df& InMinPoint, const vector3df& InMaxPoint)
	: MinPoint(InMinPoint)
	, MaxPoint(InMaxPoint)
{
	BBox.MinEdge = MinPoint;
	BBox.MaxEdge = MaxPoint;
}

FBoxSDF::~FBoxSDF()
{}

float FBoxSDF::SampleSDFByPosition(const vector3df& Position)
{
	vector3df D0 = Position - MinPoint;
	vector3df D1 = MaxPoint - Position;

	float MinDistance = TMath::Min(
		TMath::Min3(TMath::Abs(D0.X), TMath::Abs(D0.Y), TMath::Abs(D0.Z)),
		TMath::Min3(TMath::Abs(D1.X), TMath::Abs(D1.Y), TMath::Abs(D1.Z)));

	if (D0.X > 0 && D0.Y > 0 && D0.Z > 0 &&
		D1.X > 0 && D1.Y > 0 && D1.Z > 0)
	{
		// Inside box
		return Inverted ? MinDistance : -MinDistance;
	}
	else
	{
		// Outside box
		return Inverted ? -MinDistance : MinDistance;
	}
}