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
		EBF_SRC_ALPHA_SATURATE
	};

	enum E_BLEND_EQUATION
	{
		EBE_FUNC_ADD,
		EBE_FUNC_SUBTRACT,
		EBE_FUNC_REVERSE_SUBTRACT,
		EBE_MIN,
		EBE_MAX
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
		EFM_WIREFRAME = 0,
		EFM_SOLID = 1,
	};

	enum E_FRONT_FACE
	{
		EFF_CW,
		EFF_CCW
	};

	enum E_CULL_MODE
	{
		ECM_FRONT = 0,
		ECM_BACK = 1
	};
	struct TRasterizerDesc
	{
		uint8 FillMode;
		uint8 CullMode;
		uint8 MultiSampleCount;
		uint8 Reserved;
		float DepthBias;

		TRasterizerDesc()
			: FillMode(EFM_SOLID)
			, CullMode(ECM_BACK)
			, MultiSampleCount(0)
			, Reserved(0)
			, DepthBias(0.f)
		{}
	};

	// Depth and Stencil definition
	enum E_COMPARISON_FUNC
	{
		ECF_NEVER = 1,
		ECF_LESS = 2,
		ECF_LESS_EQUAL = 3,
		ECF_EQUAL = 4,
		ECF_GREATER = 5,
		ECF_NOT_EQUAL = 6,
		ECF_GREATER_EQUAL = 7,
		ECF_ALWAYS = 8
	};

	enum E_STENCIL_OP
	{
		ESO_KEEP = 1,
		ESO_ZERO = 2,
		ESO_REPLACE = 3,
		ESO_INCR_SAT = 4,
		ESO_DECR_SAT = 5,
		ESO_INVERT = 6,
		ESO_INCR = 7,
		ESO_DECR = 8
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

	//typedef struct D3D12_GRAPHICS_PIPELINE_STATE_DESC
	//{
	//	ID3D12RootSignature *pRootSignature;
	//	D3D12_SHADER_BYTECODE VS;
	//	D3D12_SHADER_BYTECODE PS;
	//	D3D12_SHADER_BYTECODE DS;
	//	D3D12_SHADER_BYTECODE HS;
	//	D3D12_SHADER_BYTECODE GS;
	//	D3D12_STREAM_OUTPUT_DESC StreamOutput;
	//	D3D12_BLEND_DESC BlendState;
	//	UINT SampleMask;
	//	D3D12_RASTERIZER_DESC RasterizerState;
	//	D3D12_DEPTH_STENCIL_DESC DepthStencilState;
	//	D3D12_INPUT_LAYOUT_DESC InputLayout;
	//	D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
	//	D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
	//	UINT NumRenderTargets;
	//	DXGI_FORMAT RTVFormats[8];
	//	DXGI_FORMAT DSVFormat;
	//	DXGI_SAMPLE_DESC SampleDesc;
	//	UINT NodeMask;
	//	D3D12_CACHED_PIPELINE_STATE CachedPSO;
	//	D3D12_PIPELINE_STATE_FLAGS Flags;
	//} 	D3D12_GRAPHICS_PIPELINE_STATE_DESC;
	struct TPipelineDesc
	{
		TString VsName;
		TString PsName;
		TString DsName;
		TString HsName;
		TString GsName;
		uint32 Flags;
		TBlendState BlendState;
		TRasterizerDesc RasterizerDesc;
		TDepthStencilDesc DepthStencilDesc;
		uint32 VsFormat;

		TPipelineDesc()
			: Flags(EPSO_DEPTH | EPSO_DEPTH_TEST)
			, VsFormat(EVSSEG_POSITION)
		{}
	};

	class FPipeline;
	typedef TI_INTRUSIVE_PTR(FPipeline) FPipelinePtr;

	class TPipeline : public TResource
	{
	public:
		TPipeline();
		virtual ~TPipeline();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		TPipelineDesc Desc;
		FPipelinePtr PipelineResource;
	protected:

	protected:

	};

	typedef TI_INTRUSIVE_PTR(TPipeline) TPipelinePtr;

}
