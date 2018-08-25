/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RT_COLOR_BUFFER
	{
		ERTC_INVALID = -1,
		ERTC_COLOR0 = 0,
		ERTC_COLOR1,
		ERTC_COLOR2,
		ERTC_COLOR3,

		ERTC_COUNT,

		ERTC_COLOR_ATTACHMENT = ((1 << ERTC_COLOR0) | (1 << ERTC_COLOR1) | (1 << ERTC_COLOR2) | (1 << ERTC_COLOR3)),
	};

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
	};

	enum E_RT_STORE_ACTION
	{
		ERT_STORE_DONTCARE,
		ERT_STORE_STORE,
		ERT_STORE_MULTISAMPLE_RESOLVE,
	};

	class TRenderTarget : public TResource
	{
	public:
		static TI_API TRenderTargetPtr Create(int32 W, int32 H);

		TRenderTarget(int32 W, int32 H);
		virtual ~TRenderTarget();

		struct RTBuffer
		{
			TTexturePtr Texture;
			E_RT_COLOR_BUFFER BufferIndex;
			E_RT_ATTACH_TYPE BufferType;
			int32 Level;

			RTBuffer()
				: BufferIndex(ERTC_INVALID)
				, BufferType(ERTAT_TEXTURE)
				, Level(0)
			{}
		};

		const RTBuffer& GetColorBuffer(int32 ColorBufferIndex)
		{
			TI_ASSERT(ColorBufferIndex >= ERTC_COLOR0 && ColorBufferIndex < ERTC_COUNT);
			return RTColorBuffers[ColorBufferIndex];
		}

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		TI_API virtual void AddColorBuffer(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBufferIndex);
		TI_API virtual void AddColorBuffer(TTexturePtr Texture, E_RT_COLOR_BUFFER ColorBufferIndex);
		TI_API virtual void AddDepthStencilBuffer(E_PIXEL_FORMAT Format);
		TI_API virtual void Compile();

		FRenderTargetPtr RTResource;
	protected:

	protected:
		vector2di Demension;

		RTBuffer RTColorBuffers[ERTC_COUNT];
		RTBuffer RTDepthStencilBuffer;
	};
}
