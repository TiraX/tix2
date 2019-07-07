/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	// Frame resources hold all the resources(textures, buffers, shaders etc), used in this frame, make sure they are not released until GPU done with them
	class FFrameResourcesDx12 : public FFrameResources
	{
	public:
		FFrameResourcesDx12();
		virtual ~FFrameResourcesDx12();

		virtual void RemoveAllReferences();

		void HoldDxReference(ComPtr<ID3D12Resource> Res);

	private:
		// Hold some temp resources used in this frame
		TList< ComPtr<ID3D12Resource> > D3d12Resources;
	};
}

#endif //COMPILE_WITH_RHI_DX12
