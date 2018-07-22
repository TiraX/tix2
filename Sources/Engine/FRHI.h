/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RHI_TYPE
	{
		ERHI_DX12 = 0,

		ERHI_NUM,
	};

	class FFrameResources;
	// Render hardware interface
	class FRHI
	{
	public: 
		static const int32 FrameBufferNum = 3;	// Use triple buffers

		static FRHI* Get();
		static void CreateRHI(E_RHI_TYPE RhiType);
		static void ReleaseRHI();

		virtual void BeginFrame() = 0;
		virtual void EndFrame() = 0;

		virtual FTexturePtr CreateTexture() = 0;
		virtual FTexturePtr CreateTexture(E_PIXEL_FORMAT Format, int32 Width, int32 Height) = 0;

		virtual FMeshBufferPtr CreateMeshBuffer() = 0;

		virtual bool UpdateHardwareBuffer(FMeshBufferPtr MeshBuffer, TMeshBufferPtr InMeshData) = 0;
		virtual bool UpdateHardwareBuffer(FTexturePtr Texture, TTexturePtr InTexData) = 0;

		E_RHI_TYPE GetRHIType() const
		{
			return RHIType;
		}

	protected:
		static FRHI* RHI;
		FRHI(E_RHI_TYPE InRHIType);
		virtual ~FRHI();

	protected:
		E_RHI_TYPE RHIType;
		FFrameResources * FrameResources[FrameBufferNum];
	};
}
