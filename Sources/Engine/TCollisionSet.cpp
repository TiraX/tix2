/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TCollisionSet.h"

namespace tix
{
	TCollisionSet::TCollisionSet()
		: TResource(ERES_COLLISION)
	{
	}

	TCollisionSet::~TCollisionSet()
	{
	}

	void TCollisionSet::InitRenderThreadResource()
	{
	}

	void TCollisionSet::DestroyRenderThreadResource()
	{
	}

	TMeshBufferPtr TCollisionSet::ConvertToMesh() const
	{
		TMeshBufferPtr Mesh = ti_new TMeshBuffer;

		TVector<vector3df> Positions;
		TVector<uint16> Indices;
		Positions.reserve(64 * 1024);
		Indices.reserve(128 * 1024);

		for (const auto& Sphere : Spheres)
		{
			TShape::CreateICOSphere(3, Sphere.Center, Sphere.Radius, Positions, Indices);
		}

		for (const auto& Box : Boxes)
		{
			TShape::CreateBox(Box.Center, Box.Edge, Box.Rotation, Positions, Indices);
		}

		for (const auto& Capsule : Capsules)
		{
			TShape::CreateCapsule(6, 10, Capsule.Center, Capsule.Radius, Capsule.Length, Capsule.Rotation, Positions, Indices);
		}

		for (const auto& Convex : Convexes)
		{
			uint16 IndexOffset = (uint16)Positions.size();
			Positions.insert(Positions.end(), Convex.VertexData.begin(), Convex.VertexData.end());
			for (auto I : Convex.IndexData)
			{
				uint16 Index = I + IndexOffset;
				Indices.push_back(Index);
			}
		}
		TI_ASSERT(Positions.size() < 65535);
		Mesh->SetVertexStreamData(EVSSEG_POSITION, Positions.data(), (uint32)Positions.size(), EIT_16BIT, Indices.data(), (uint32)Indices.size());

		return Mesh;
	}
}