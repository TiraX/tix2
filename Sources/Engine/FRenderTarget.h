/*
	TiX Engine v2.0 Copyright (C) 2018
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
	};

	enum E_RT_STORE_ACTION
	{
		ERT_STORE_DONTCARE,
		ERT_STORE_STORE,
		ERT_STORE_MULTISAMPLE_RESOLVE,
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

		virtual void Destroy() {};

		struct RTBuffer
		{
			FTexturePtr Texture;
			//FRenderTargetResourcePtr RTResource;
			E_RT_COLOR_BUFFER BufferIndex;

			RTBuffer()
				: BufferIndex(ERTC_INVALID)
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

		const FRenderResourceTable& GetRTColorTable() const
		{
			return RTColorTable;
		}

		const FRenderResourceTable& GetRTDepthTable() const
		{
			return RTDepthTable;
		}

		TI_API virtual void AddColorBuffer(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBufferIndex);
		TI_API virtual void AddColorBuffer(FTexturePtr Texture, E_RT_COLOR_BUFFER ColorBufferIndex);
		TI_API virtual void AddDepthStencilBuffer(E_PIXEL_FORMAT Format);
		TI_API virtual void Compile();

	protected:

	protected:
		vector2di Demension;

		RTBuffer RTColorBuffers[ERTC_COUNT];
		RTBuffer RTDepthStencilBuffer;

		FRenderResourceTable RTColorTable;
		FRenderResourceTable RTDepthTable;

		int32 ColorBuffers;
		friend class FRHI;
	};
}
