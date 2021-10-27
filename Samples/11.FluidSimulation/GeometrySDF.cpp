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
{
	Center = (InMinPoint + InMaxPoint) * 0.5f;
	EdgeLengthHalf = (InMaxPoint - InMinPoint) * 0.5f;
}

FBoxSDF::~FBoxSDF()
{}

float FBoxSDF::SampleSDFByPosition(const vector3df& Position)
{
	vector3df P = Position - Center;
	P.X = TMath::Abs(P.X);
	P.Y = TMath::Abs(P.Y);
	P.Z = TMath::Abs(P.Z);
	vector3df Q = P - EdgeLengthHalf;

	float DisOutside = vector3df(TMath::Max(Q.X, 0.f), TMath::Max(Q.Y, 0.f), TMath::Max(Q.Z, 0.f)).getLength();
	float Distance = DisOutside + TMath::Min(TMath::Max3(Q.X, Q.Y, Q.Z), 0.f);

	return Inverted ? -Distance : Distance;
}