/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RT_ATTACH_TYPE
	{
		ERTAT_TEXTURE,
		ERTAT_TEXTURE_CUBE,
		ERTAT_RENDERBUFFER,

		ERTAT_COUNT,
	};

	enum E_RT_FLAG
	{
		ERTF_COMPILED = 1 << 0,
	};

	enum E_RT_LOAD_ACTION
	{
		ERT_LOAD_DONTCARE,
		ERT_LOAD_LOAD,
		ERT_LOAD_CLEAR,
        
        ERT_LOAD_ACTION_NUM,
	};

	enum E_RT_STORE_ACTION
	{
		ERT_STORE_DONTCARE,
		ERT_STORE_STORE,
		ERT_STORE_MULTISAMPLE_RESOLVE,
        ERT_STORE_STORE_AND_MULTISAMPLE_RESOLVE,
        
        ERT_STORE_ACTION_NUM,
	};

	//class FRenderTargetResource : public FRenderResource
	//{
	//public:
	//	FRenderTargetResource()
	//	{}
	//	~FRenderTargetResource()
	//	{}

	//	virtual void Destroy() override {};
	//private:

	//};

	class FRenderTarget : public FRenderResource
	{
	public:
		TI_API static FRenderTargetPtr Create(int32 W, int32 H);

		FRenderTarget(int32 W, int32 H);
		virtual ~FRenderTarget();

		struct RTBuffer
		{
			FTexturePtr Texture;
			//FRenderTargetResourcePtr RTResource;
			E_RT_COLOR_BUFFER BufferIndex;
			E_RT_LOAD_ACTION LoadAction;
			E_RT_STORE_ACTION StoreAction;

			RTBuffer()
				: BufferIndex(ERTC_INVALID)
				, LoadAction(ERT_LOAD_DONTCARE)
				, StoreAction(ERT_STORE_DONTCARE)
			{}
			~RTBuffer()
			{
				Texture = nullptr;
				//RTResource = nullptr;
			}
		};

		const vector2di& GetDemension() const
		{
			return Demension;
		}

		int32 GetColorBufferCount() const
		{
			return ColorBuffers;
		}

		const RTBuffer& GetColorBuffer(int32 ColorBufferIndex)
		{
			TI_ASSERT(ColorBufferIndex >= ERTC_COLOR0 && ColorBufferIndex < ERTC_COUNT);
			return RTColorBuffers[ColorBufferIndex];
		}
		const RTBuffer& GetDepthStencilBuffer()
		{
			return RTDepthStencilBuffer;
		}

		TI_API virtual void AddColorBuffer(E_PIXEL_FORMAT Format, uint32 Mips, E_RT_COLOR_BUFFER ColorBufferIndex, E_RT_LOAD_ACTION LoadAction, E_RT_STORE_ACTION StoreAction);
		TI_API virtual void AddColorBuffer(FTexturePtr Texture, E_RT_COLOR_BUFFER ColorBufferIndex, E_RT_LOAD_ACTION LoadAction, E_RT_STORE_ACTION StoreAction);
		TI_API virtual void AddDepthStencilBuffer(E_PIXEL_FORMAT Format, uint32 Mips, E_RT_LOAD_ACTION LoadAction, E_RT_STORE_ACTION StoreAction);
		TI_API virtual void AddDepthStencilBuffer(FTexturePtr Texture, E_RT_LOAD_ACTION LoadAction, E_RT_STORE_ACTION StoreAction);
        
        // For metal tile shader
        TI_API virtual void SetTileSize(const vector2di& InTileSize)
        {}
        TI_API virtual void SetThreadGroupMemoryLength(uint32 Length)
        {}

        TI_API virtual void Compile();
	protected:

	protected:
		vector2di Demension;

		RTBuffer RTColorBuffers[ERTC_COUNT];
		RTBuffer RTDepthStencilBuffer;

		int32 ColorBuffers;
		friend class FRHI;
	};
}
