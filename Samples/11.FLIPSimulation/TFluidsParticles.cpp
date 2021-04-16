/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TFluidsParticles.h"
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
using namespace rapidjson;

TFluidsParticles::TFluidsParticles()
	: Radius(0)
{
}

TFluidsParticles::~TFluidsParticles()
{
}
//! returns a vector with each component between 0.0 ~ 1.0, un-normalized
static inline vectype RandomVector()
{
	const ftype k_inv = 1.0f / RAND_MAX;
	return vectype(rand() * k_inv, rand() * k_inv, rand() * k_inv);
}

void TFluidsParticles::InitWithShapeSphere(const vectype& InCenter, float InRadius, float InSeperation)
{
	vectype Min = InCenter - vectype(InRadius, InRadius, InRadius);
	vectype Max = InCenter + vectype(InRadius, InRadius, InRadius);

	Radius = InSeperation * 0.5f;

	int MaxCount = int(InRadius * 2.f / InSeperation);
	MaxCount = MaxCount * MaxCount * MaxCount;
	Particles.reserve(MaxCount);

	const ftype JitterScale = 0.5f;
	TMath::RandSeed(3499);

	float RadiusSQ = InRadius * InRadius;
	for (ftype z = Min.Z; z <= Max.Z; z += InSeperation)
	{
		for (ftype y = Min.Y; y <= Max.Y; y += InSeperation)
		{
			for (ftype x = Min.X; x <= Max.X; x += InSeperation)
			{
				vectype Pos = vectype(x, y, z);
				if ((InCenter - Pos).getLengthSQ() < RadiusSQ)
				{
					vectype Jitter = RandomVector() * 2.f - vectype(1.f, 1.f, 1.f);
					Jitter *= InSeperation * JitterScale;
					Pos += Jitter;

					TParticle P;
					P.Position = Pos;
					Particles.push_back(P);
				}
			}
		}
	}
}

void TFluidsParticles::SearchParticlesNear(int32 ParticleIndex, ftype Radius, TVector<int32>& ParticleIndicesInRadius)
{
	const TParticle& P = Particles[ParticleIndex];
	const ftype RSQ = Radius * Radius;

	const int32 Num = (int32)Particles.size();
	for (int32 i = 0; i < Num; i++)
	{
		if (i == ParticleIndex)
			continue;

		if ((Particles[i].Position - P.Position).getLengthSQ() < RSQ)
			ParticleIndicesInRadius.push_back(i);
	}
}

//node = hou.pwd()
//geo = node.geometry()
//
//# Add code to modify contents of geo.
//# Use drop down menu to select examples.
//
//import json
//
//
//with open("d:/test.json", 'r') as f :
//	js = json.load(f)
//	print(js['count'])
//	print(js['positions'][1])
void TFluidsParticles::ExportToJson(const TString& Filename)
{
	Document JsonDoc;
	Value ArrayPosition(kArrayType);
	Value ArrayVelocity(kArrayType);
	Document::AllocatorType& Allocator = JsonDoc.GetAllocator();

	JsonDoc.SetObject();
	ArrayPosition.Reserve((uint32)Particles.size() * 3, Allocator);
	ArrayVelocity.Reserve((uint32)Particles.size() * 3, Allocator);
	for (uint32 i = 0; i < Particles.size(); i++)
	{
		ArrayPosition.PushBack(Particles[i].Position.X, Allocator);
		ArrayPosition.PushBack(Particles[i].Position.Y, Allocator);
		ArrayPosition.PushBack(Particles[i].Position.Z, Allocator);

		ArrayVelocity.PushBack(Particles[i].Velocity.X, Allocator);
		ArrayVelocity.PushBack(Particles[i].Velocity.Y, Allocator);
		ArrayVelocity.PushBack(Particles[i].Velocity.Z, Allocator);
	}

	Value TotalParticles(kNumberType);
	TotalParticles.SetInt((int32)Particles.size());
	JsonDoc.AddMember("count", TotalParticles, Allocator);
	JsonDoc.AddMember("positions", ArrayPosition, Allocator);
	JsonDoc.AddMember("velocities", ArrayVelocity, Allocator);

	StringBuffer StrBuffer;
	Writer<StringBuffer> JsonWriter(StrBuffer);
	JsonDoc.Accept(JsonWriter);

	TFile Output;
	if (Output.Open(Filename, EFA_CREATEWRITE))
	{
		Output.Write(StrBuffer.GetString(), (int32)StrBuffer.GetSize());
		Output.Close();
	}
}