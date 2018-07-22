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
		EPSO_STENCIL_TEST = 1 << 4,

		//EPSO_FILLMODE = 1 << 0,
		//EPSO_CULL = 1 << 1,
		//EPSO_DEPTH = 1 << 2,
		//EPSO_DEPTH_TEST = 1 << 3,
		////EPSO_STENCIL	= 1 << 4,
		//EPSO_STENCIL_TEST = 1 << 5,
		//EPSO_BLEND = 1 << 6,
		//EPSO_FOG = 1 << 7,
		//EPSO_SCISSOR = 1 << 8,
		//EPSO_ALPHA_TEST = 1 << 9,
		//EPSO_IGNORE_DEPTH_BUFFER = 1 << 10,	// for metal, some shaders do not need depth buffer
	};

	//typedef struct D3D12_RENDER_TARGET_BLEND_DESC
	//{
	//	BOOL BlendEnable;
	//	BOOL LogicOpEnable;
	//	D3D12_BLEND SrcBlend;
	//	D3D12_BLEND DestBlend;
	//	D3D12_BLEND_OP BlendOp;
	//	D3D12_BLEND SrcBlendAlpha;
	//	D3D12_BLEND DestBlendAlpha;
	//	D3D12_BLEND_OP BlendOpAlpha;
	//	D3D12_LOGIC_OP LogicOp;
	//	UINT8 RenderTargetWriteMask;
	//} 	D3D12_RENDER_TARGET_BLEND_DESC;
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
	};

	//typedef struct D3D12_RASTERIZER_DESC
	//{
	//	D3D12_FILL_MODE FillMode;
	//	D3D12_CULL_MODE CullMode;
	//	BOOL FrontCounterClockwise;
	//	INT DepthBias;
	//	FLOAT DepthBiasClamp;
	//	FLOAT SlopeScaledDepthBias;
	//	BOOL DepthClipEnable;
	//	BOOL MultisampleEnable;
	//	BOOL AntialiasedLineEnable;
	//	UINT ForcedSampleCount;
	//	D3D12_CONSERVATIVE_RASTERIZATION_MODE ConservativeRaster;
	//} 	D3D12_RASTERIZER_DESC;
	struct TRasterizerDesc
	{
		uint8 FillMode;
		uint8 CullMode;
		uint8 MultiSampleCount;
		uint8 Reserved;
		float DepthBias;
	};

	//typedef struct D3D12_DEPTH_STENCILOP_DESC
	//{
	//	D3D12_STENCIL_OP StencilFailOp;
	//	D3D12_STENCIL_OP StencilDepthFailOp;
	//	D3D12_STENCIL_OP StencilPassOp;
	//	D3D12_COMPARISON_FUNC StencilFunc;
	//} 	D3D12_DEPTH_STENCILOP_DESC;

	//typedef struct D3D12_DEPTH_STENCIL_DESC
	//{
	//	BOOL DepthEnable;
	//	D3D12_DEPTH_WRITE_MASK DepthWriteMask;
	//	D3D12_COMPARISON_FUNC DepthFunc;
	//	BOOL StencilEnable;
	//	UINT8 StencilReadMask;
	//	UINT8 StencilWriteMask;
	//	D3D12_DEPTH_STENCILOP_DESC FrontFace;
	//	D3D12_DEPTH_STENCILOP_DESC BackFace;
	//} 	D3D12_DEPTH_STENCIL_DESC;

	struct TDepthStencilOpDesc
	{
		uint8 StencilFailOp;
		uint8 StencilDepthFailOp;
		uint8 StencilPassOp;
		uint8 StencilFunc;
	};
	struct TDepthStencilDesc
	{
		uint8 DepthFunc;
		uint8 StencilReadMask;
		uint8 StencilWriteMask;
		uint8 Reserved;
		TDepthStencilOpDesc FrontFace;
		TDepthStencilOpDesc BackFace;
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

		FPipelinePtr PipelineResource;
	protected:

	protected:

	};

	typedef TI_INTRUSIVE_PTR(TPipeline) TPipelinePtr;

}
