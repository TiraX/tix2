/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_PIPELINE_STATES_OPTION	//This is the enable/disable options
	{
		EPSO_START = 1,

		EPSO_BLEND = 1 << 0,
		EPSO_DEPTH = 1 << 1,
		EPSO_DEPTH_TEST = 1 << 2,
		EPSO_STENCIL = 1 << 3,
		EPSO_STENCIL_TEST = 1 << 4
	};
	enum E_BLEND_FUNC
	{
		EBF_ZERO,
		EBF_ONE,
		EBF_SRC_COLOR,
		EBF_ONE_MINUS_SRC_COLOR,
		EBF_DEST_COLOR,
		EBF_ONE_MINUS_DEST_COLOR,
		EBF_SRC_ALPHA,
		EBF_ONE_MINUS_SRC_ALPHA,
		EBF_DST_ALPHA,
		EBF_ONE_MINUS_DST_ALPHA,
		EBF_CONSTANT_COLOR,
		EBF_ONE_MINUS_CONSTANT_COLOR,
		EBF_CONSTANT_ALPHA,
		EBF_ONE_MINUS_CONSTANT_ALPHA,
		EBF_SRC_ALPHA_SATURATE,

		EBF_COUNT,
	};

	enum E_BLEND_EQUATION
	{
		EBE_FUNC_ADD,
		EBE_FUNC_SUBTRACT,
		EBE_FUNC_REVERSE_SUBTRACT,
		EBE_MIN,
		EBE_MAX,

		EBE_COUNT,
	};

	struct TBlendState
	{
		uint8 SrcBlend;
		uint8 DestBlend;
		uint8 BlendOp;
		uint8 SrcBlendAlpha;
		uint8 DestBlendAlpha;
		uint8 BlendOpAlpha;
		uint8 LogicOp;
		uint8 RenderTargetWriteMask;

		TBlendState()
			: SrcBlend(EBF_SRC_ALPHA)
			, DestBlend(EBF_ONE_MINUS_SRC_ALPHA)
			, BlendOp(EBE_FUNC_ADD)
			, SrcBlendAlpha(EBF_ONE)
			, DestBlendAlpha(EBF_ZERO)
			, BlendOpAlpha(EBE_FUNC_ADD)
			, LogicOp(0)
			, RenderTargetWriteMask(0)
		{}
	};

	// Rasterizer definition
	enum E_FILL_MODE
	{
		EFM_WIREFRAME,
		EFM_SOLID,

		EFM_COUNT,
	};

	enum E_FRONT_FACE
	{
		EFF_CW,
		EFF_CCW,

		EFF_COUNT,
	};

	enum E_CULL_MODE
	{
		ECM_NONE,
		ECM_FRONT,
		ECM_BACK,

		ECM_COUNT,
	};
	struct TRasterizerDesc
	{
		uint8 FillMode;
		uint8 CullMode;
		uint8 MultiSampleCount;
		uint8 Reserved;
		int32 DepthBias;

		TRasterizerDesc()
			: FillMode(EFM_SOLID)
			, CullMode(ECM_BACK)
			, MultiSampleCount(0)
			, Reserved(0)
			, DepthBias(0)
		{}
	};

	// Depth and Stencil definition
	enum E_COMPARISON_FUNC
	{
		ECF_NEVER,
		ECF_LESS,
		ECF_LESS_EQUAL,
		ECF_EQUAL,
		ECF_GREATER,
		ECF_NOT_EQUAL,
		ECF_GREATER_EQUAL,
		ECF_ALWAYS,

		ECF_COUNT,
	};

	enum E_STENCIL_OP
	{
		ESO_KEEP,
		ESO_ZERO,
		ESO_REPLACE,
		ESO_INCR_SAT,
		ESO_DECR_SAT,
		ESO_INVERT,
		ESO_INCR,
		ESO_DECR,

		ESO_COUNT,
	};
	struct TDepthStencilOpDesc
	{
		uint8 StencilFailOp;
		uint8 StencilDepthFailOp;
		uint8 StencilPassOp;
		uint8 StencilFunc;

		TDepthStencilOpDesc()
			: StencilFailOp(ESO_KEEP)
			, StencilDepthFailOp(ESO_KEEP)
			, StencilPassOp(ESO_KEEP)
			, StencilFunc(ECF_ALWAYS)
		{}
	};
	struct TDepthStencilDesc
	{
		uint8 DepthFunc;
		uint8 StencilReadMask;
		uint8 StencilWriteMask;
		uint8 Reserved;
		TDepthStencilOpDesc FrontFace;
		TDepthStencilOpDesc BackFace;

		TDepthStencilDesc()
			: DepthFunc(ECF_LESS)
			, StencilReadMask(0xff)
			, StencilWriteMask(0xff)
			, Reserved(0)
		{}
	};

	enum E_SHADER_STAGE
	{
		ESS_VERTEX_SHADER,
		ESS_PIXEL_SHADER,
		ESS_DOMAIN_SHADER,
		ESS_HULL_SHADER,
		ESS_GEOMETRY_SHADER,

		ESS_COUNT,
	};
	struct TPipelineDesc
	{
		TString ShaderName[ESS_COUNT];
		uint32 Flags;
		TBlendState BlendState;
		TRasterizerDesc RasterizerDesc;
		TDepthStencilDesc DepthStencilDesc;
		uint32 VsFormat;
		uint32 PrimitiveType;

		E_PIXEL_FORMAT RTFormats[FRHIConfig::MultiRTMax];
		E_PIXEL_FORMAT DepthFormat;

		TPipelineDesc()
			: Flags(EPSO_DEPTH | EPSO_DEPTH_TEST)
			, VsFormat(EVSSEG_POSITION)
			, PrimitiveType(EPT_TRIANGLELIST)
		{
			for (int32 i = 0; i < FRHIConfig::MultiRTMax; i++)
			{
				RTFormats[i] = FRHIConfig::DefaultBackBufferFormat;
			}
			DepthFormat = FRHIConfig::DefaultDepthBufferFormat;
		}

		bool IsEnabled(E_PIPELINE_STATES_OPTION Option) const
		{
			return (Flags & Option) != 0;
		}
	};

	class TPipeline : public TResource
	{
	public:
		TPipeline();
		virtual ~TPipeline();

		// a test interface.
		static TI_API TPipelinePtr CreatePipeline(const TString& VsPath, const TString& PsPath, const uint32 VsFormat, E_CULL_MODE CullMode);

		void SetShader(E_SHADER_STAGE ShaderStage, const TString& ShaderName, const int8* InShaderCode, int32 CodeLength);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		TPipelineDesc Desc;
		FPipelinePtr PipelineResource;

		TStream ShaderCode[ESS_COUNT];
	protected:

	protected:

	};
}
