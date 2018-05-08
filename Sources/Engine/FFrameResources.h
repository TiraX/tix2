/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// Frame resources hold all the resources(textures, buffers, shaders etc), used in this frame, make sure they are not released until GPU done with them
	class FFrameResources
	{
	public:
		static const int32 DefaultReserveCount = 512;

		FFrameResources();
		virtual ~FFrameResources();

		virtual void RemoveAllReferences();

		void HoldReference(FMeshBufferPtr MeshBuffer);
	private:
		TVector<FMeshBufferPtr> MeshBuffers;
	};
}
