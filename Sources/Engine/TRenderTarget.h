/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_RT_ATTACH
	{
		ERTA_COLOR0,
		ERTA_COLOR1,
		ERTA_COLOR2,
		ERTA_COLOR3,
		ERTA_DEPTH,
		ERTA_STENCIL,
		ERTA_DEPTH_STENCIL,

		ERTA_COUNT,

		ERTA_COLOR_ATTACHMENT = ((1 << ERTA_COLOR0) | (1 << ERTA_COLOR1) | (1 << ERTA_COLOR2) | (1 << ERTA_COLOR3)),
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

		struct RTAttachment
		{
			TTexturePtr Texture;
			E_RT_ATTACH Attachment;
			E_RT_ATTACH_TYPE AttachType;
			int32 Level;
		};

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		TI_API virtual void AddTextureAttachment(E_PIXEL_FORMAT Format, E_RT_ATTACH Attachment);
		TI_API virtual void AddAttachment(TTexturePtr Texture, E_RT_ATTACH Attachment);
		TI_API virtual void Compile();

		FRenderTargetPtr RTResource;
	protected:

	protected:
		vector2di Demension;

		typedef TVector<RTAttachment> VecRTAttachments;
		VecRTAttachments RtAttachments;
	};
}
