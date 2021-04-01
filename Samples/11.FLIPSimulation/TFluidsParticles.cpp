/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TFluidsParticles.h"

TFluidsParticles::TFluidsParticles()
{
}

TFluidsParticles::~TFluidsParticles()
{
}
//! returns a vector with each component between 0.0 ~ 1.0, un-normalized
static inline vector3df RandomVector()
{
	const float k_inv = 1.0f / RAND_MAX;
	return vector3df(rand() * k_inv, rand() * k_inv, rand() * k_inv);
}

void TFluidsParticles::InitWithShapeSphere(const vector3df& InCenter, float InRadius, float InSeperation)
{
	vector3df Min = InCenter - vector3df(InRadius, InRadius, InRadius);
	vector3df Max = InCenter + vector3df(InRadius, InRadius, InRadius);

	int MaxCount = int(InRadius * 2.f / InSeperation);
	MaxCount = MaxCount * MaxCount * MaxCount;
	Positions.reserve(MaxCount);
	Velocities.reserve(MaxCount);

	const float JitterScale = 0.1f;
	TMath::RandSeed(3499);

	float RadiusSQ = InRadius * InRadius;
	for (float z = Min.Z; z <= Max.Z; z += InSeperation)
	{
		for (float y = Min.Y; y <= Max.Y; y += InSeperation)
		{
			for (float x = Min.X; x <= Max.X; x += InSeperation)
			{
				vector3df Pos = vector3df(x, y, z);
				if ((InCenter - Pos).getLengthSQ() < RadiusSQ)
				{
					vector3df Jitter = RandomVector() * 2.f - vector3df(1.f, 1.f, 1.f);
					Jitter *= InSeperation * JitterScale;
					Pos += Jitter;
					Positions.push_back(Pos);
					Velocities.push_back(vector3df());
				}
			}
		}
	}
}