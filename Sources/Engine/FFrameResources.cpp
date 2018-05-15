/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FRHI.h"
#include "FFrameResources.h"

namespace tix
{
	FFrameResources::FFrameResources()
	{
		MeshBuffers.reserve(DefaultReserveCount);
	}

	FFrameResources::~FFrameResources()
	{
		RemoveAllReferences();
	}

	void FFrameResources::RemoveAllReferences()
	{
		for (auto& MB : MeshBuffers)
		{
			MB = nullptr;
		}
		MeshBuffers.clear();
		for (auto& Tex : Textures)
		{
			Tex = nullptr;
		}
		Textures.clear();
	}

	void FFrameResources::HoldReference(FMeshBufferPtr MeshBuffer)
	{
		MeshBuffers.push_back(MeshBuffer);
	}

	void FFrameResources::HoldReference(FTexturePtr Texture)
	{
		Textures.push_back(Texture);
	}
}