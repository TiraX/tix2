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
	Particles.reserve(MaxCount);

	const float JitterScale = 0.5f;
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

					TParticle P;
					P.Position = Pos;
					Particles.push_back(P);
				}
			}
		}
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
	ArrayPosition.Reserve(Particles.size() * 3, Allocator);
	ArrayVelocity.Reserve(Particles.size() * 3, Allocator);
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
	TotalParticles.SetInt(Particles.size());
	JsonDoc.AddMember("count", TotalParticles, Allocator);
	JsonDoc.AddMember("positions", ArrayPosition, Allocator);
	JsonDoc.AddMember("velocities", ArrayVelocity, Allocator);

	StringBuffer StrBuffer;
	Writer<StringBuffer> JsonWriter(StrBuffer);
	JsonDoc.Accept(JsonWriter);

	TFile Output;
	if (Output.Open(Filename, EFA_CREATEWRITE))
	{
		Output.Write(StrBuffer.GetString(), StrBuffer.GetSize());
		Output.Close();
	}
}