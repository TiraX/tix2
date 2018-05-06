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

		virtual bool UpdateHardwareBuffer(FMeshBufferPtr MeshBuffer) = 0;

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
