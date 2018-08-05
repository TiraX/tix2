/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12

namespace tix
{
	static const DXGI_FORMAT k_PIXEL_FORMAT_MAP[] = 
	{
		DXGI_FORMAT_UNKNOWN, //EPF_A8,
		DXGI_FORMAT_UNKNOWN, //EPF_RGB8,
		DXGI_FORMAT_UNKNOWN, //EPF_BGR8,
		DXGI_FORMAT_R8G8B8A8_UNORM, //EPF_RGBA8,
		DXGI_FORMAT_B8G8R8A8_UNORM, //EPF_BGRA8,
		DXGI_FORMAT_UNKNOWN, //EPF_SRGB8,
		DXGI_FORMAT_UNKNOWN, //EPF_SRGB8_ALPHA,

		// Float formats
		DXGI_FORMAT_UNKNOWN, //EPF_R16F,
		DXGI_FORMAT_UNKNOWN, //EPF_RG16F,
		DXGI_FORMAT_UNKNOWN, //EPF_RGB16F,
		DXGI_FORMAT_UNKNOWN, //EPF_RGBA16F,
		DXGI_FORMAT_UNKNOWN, //EPF_R32F,
		DXGI_FORMAT_UNKNOWN, //EPF_RG32F,
		DXGI_FORMAT_UNKNOWN, //EPF_RGB32F,
		DXGI_FORMAT_UNKNOWN, //EPF_RGBA32F,

		// Depth formats
		DXGI_FORMAT_UNKNOWN, //EPF_DEPTH16,
		DXGI_FORMAT_UNKNOWN, //EPF_DEPTH24,
		DXGI_FORMAT_D32_FLOAT, //EPF_DEPTH32,
		DXGI_FORMAT_D24_UNORM_S8_UINT, //EPF_DEPTH24_STENCIL8,

		// Compressed formats
		// DXT formats
		DXGI_FORMAT_BC1_UNORM,	//EPF_DDS_DXT1,
		DXGI_FORMAT_BC2_UNORM,	//EPF_DDS_DXT3,
		DXGI_FORMAT_BC3_UNORM,	//EPF_DDS_DXT5,
		DXGI_FORMAT_UNKNOWN, //EPF_DDS_RESERVED,

		// PVR formats
		DXGI_FORMAT_UNKNOWN, //EPF_COMPRESSED_sRGB_PVRTC_4BPP,
		DXGI_FORMAT_UNKNOWN, //EPF_COMPRESSED_sRGB_Alpha_PVRTC_4BPP,
		DXGI_FORMAT_UNKNOWN, //EPF_COMPRESSED_RGB_PVRTC_4BPP,
		DXGI_FORMAT_UNKNOWN, //EPF_COMPRESSED_RGBA_PVRTC_4BPP,

		// ETC format
		DXGI_FORMAT_UNKNOWN, //EPF_COMPRESSED_ETC,

		DXGI_FORMAT_UNKNOWN, //EPF_UNKNOWN,
	};

	static DXGI_FORMAT k_MESHBUFFER_STREAM_FORMAT_MAP[ESSI_TOTAL] = 
	{
		DXGI_FORMAT_R32G32B32_FLOAT,	// ESSI_POSITION,
		DXGI_FORMAT_R8G8B8A8_UNORM,		// ESSI_NORMAL,
		DXGI_FORMAT_R8G8B8A8_UNORM,	// ESSI_COLOR,
		DXGI_FORMAT_R32G32_FLOAT,	// ESSI_TEXCOORD0,	// TI_TODO("May use half float texcoord in future");
		DXGI_FORMAT_R32G32_FLOAT,	// ESSI_TEXCOORD1,	// TI_TODO("May use half float texcoord in future");
		DXGI_FORMAT_R8G8B8A8_UNORM,	// ESSI_TANGENT,	// TI_TODO("May use packed tangent in future");
		DXGI_FORMAT_R8G8B8A8_UINT,	// ESSI_BLENDINDEX,
		DXGI_FORMAT_R32G32B32A32_FLOAT,	// ESSI_BLENDWEIGHT,// TI_TODO("May use half float blend weight in future");
	};

	static D3D12_BLEND k_BLEND_FUNC_MAP[EBF_COUNT] =
	{
		D3D12_BLEND_ZERO,	//EBF_ZERO,
		D3D12_BLEND_ONE,	//EBF_ONE,
		D3D12_BLEND_SRC_COLOR,	//EBF_SRC_COLOR,
		D3D12_BLEND_INV_SRC_COLOR,	//EBF_ONE_MINUS_SRC_COLOR,
		D3D12_BLEND_DEST_COLOR,	//EBF_DEST_COLOR,
		D3D12_BLEND_INV_DEST_COLOR,	//EBF_ONE_MINUS_DEST_COLOR,
		D3D12_BLEND_SRC_ALPHA,	//EBF_SRC_ALPHA,
		D3D12_BLEND_INV_SRC_ALPHA,	//EBF_ONE_MINUS_SRC_ALPHA,
		D3D12_BLEND_DEST_ALPHA,	//EBF_DST_ALPHA,
		D3D12_BLEND_INV_DEST_ALPHA,	//EBF_ONE_MINUS_DST_ALPHA,
		D3D12_BLEND_SRC1_COLOR,	//EBF_CONSTANT_COLOR,
		D3D12_BLEND_INV_SRC1_COLOR,	//EBF_ONE_MINUS_CONSTANT_COLOR,
		D3D12_BLEND_SRC1_ALPHA,	//EBF_CONSTANT_ALPHA,
		D3D12_BLEND_INV_SRC1_ALPHA,	//EBF_ONE_MINUS_CONSTANT_ALPHA,
		D3D12_BLEND_SRC_ALPHA_SAT,	//EBF_SRC_ALPHA_SATURATE
	};

	static D3D12_BLEND_OP k_BLEND_EQUATION_MAP[EBE_COUNT] =
	{
		D3D12_BLEND_OP_ADD,	//EBE_FUNC_ADD,
		D3D12_BLEND_OP_SUBTRACT,	//EBE_FUNC_SUBTRACT,
		D3D12_BLEND_OP_REV_SUBTRACT,	//EBE_FUNC_REVERSE_SUBTRACT,
		D3D12_BLEND_OP_MIN,	//EBE_MIN,
		D3D12_BLEND_OP_MAX,	//EBE_MAX
	};

	inline void MakeDx12BlendState(const TPipelineDesc& Desc, D3D12_BLEND_DESC& BlendState)
	{
		BlendState.AlphaToCoverageEnable = FALSE;
		BlendState.IndependentBlendEnable = FALSE;

		D3D12_RENDER_TARGET_BLEND_DESC BlendDesc;
		BlendDesc.BlendEnable = Desc.IsEnabled(EPSO_BLEND);
		BlendDesc.LogicOpEnable = FALSE;
		BlendDesc.SrcBlend = k_BLEND_FUNC_MAP[Desc.BlendState.SrcBlend];
		BlendDesc.DestBlend = k_BLEND_FUNC_MAP[Desc.BlendState.DestBlend];
		BlendDesc.BlendOp = k_BLEND_EQUATION_MAP[Desc.BlendState.BlendOp];
		BlendDesc.SrcBlendAlpha = k_BLEND_FUNC_MAP[Desc.BlendState.SrcBlendAlpha];
		BlendDesc.DestBlendAlpha = k_BLEND_FUNC_MAP[Desc.BlendState.DestBlendAlpha];
		BlendDesc.BlendOpAlpha = k_BLEND_EQUATION_MAP[Desc.BlendState.BlendOpAlpha];
		BlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
		BlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		for (int32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			BlendState.RenderTarget[i] = BlendDesc;
	}

	static D3D12_COMPARISON_FUNC k_COMPARISON_FUNC_MAP[ECF_COUNT] =
	{
		D3D12_COMPARISON_FUNC_NEVER,	//ECF_NEVER,
		D3D12_COMPARISON_FUNC_LESS,	//ECF_LESS,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,	//ECF_LESS_EQUAL,
		D3D12_COMPARISON_FUNC_EQUAL,	//ECF_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER,	//ECF_GREATER,
		D3D12_COMPARISON_FUNC_NOT_EQUAL,	//ECF_NOT_EQUAL,
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,	//ECF_GREATER_EQUAL,
		D3D12_COMPARISON_FUNC_ALWAYS,	//ECF_ALWAYS,
	};
	static D3D12_STENCIL_OP k_STENCIL_OP_MAP[ESO_COUNT] =
	{
		D3D12_STENCIL_OP_KEEP,	//ESO_KEEP,
		D3D12_STENCIL_OP_ZERO,	//ESO_ZERO,
		D3D12_STENCIL_OP_REPLACE,	//ESO_REPLACE,
		D3D12_STENCIL_OP_INCR_SAT,	//ESO_INCR_SAT,
		D3D12_STENCIL_OP_DECR_SAT,	//ESO_DECR_SAT,
		D3D12_STENCIL_OP_INVERT,	//ESO_INVERT,
		D3D12_STENCIL_OP_INCR,	//ESO_INCR,
		D3D12_STENCIL_OP_DECR,	//ESO_DECR,
	};

	inline void MakeDx12DepthStencilState(const TPipelineDesc& Desc, D3D12_DEPTH_STENCIL_DESC& DepthStencilState)
	{
		DepthStencilState.DepthEnable = Desc.IsEnabled(EPSO_DEPTH);
		DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		DepthStencilState.DepthFunc = k_COMPARISON_FUNC_MAP[Desc.DepthStencilDesc.DepthFunc];
		DepthStencilState.StencilEnable = Desc.IsEnabled(EPSO_STENCIL);
		DepthStencilState.StencilReadMask = Desc.DepthStencilDesc.StencilReadMask;
		DepthStencilState.StencilWriteMask = Desc.DepthStencilDesc.StencilWriteMask;

		DepthStencilState.FrontFace.StencilFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilFailOp];
		DepthStencilState.FrontFace.StencilDepthFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilDepthFailOp];
		DepthStencilState.FrontFace.StencilPassOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilPassOp];
		DepthStencilState.FrontFace.StencilFunc = k_COMPARISON_FUNC_MAP[Desc.DepthStencilDesc.FrontFace.StencilFunc];

		DepthStencilState.BackFace.StencilFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilFailOp];
		DepthStencilState.BackFace.StencilDepthFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilDepthFailOp];
		DepthStencilState.BackFace.StencilPassOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilPassOp];
		DepthStencilState.BackFace.StencilFunc = k_COMPARISON_FUNC_MAP[Desc.DepthStencilDesc.BackFace.StencilFunc];
	}

	static D3D12_FILL_MODE k_FILL_MODE_MAP[EFM_COUNT] =
	{
		D3D12_FILL_MODE_WIREFRAME,	//EFM_WIREFRAME,
		D3D12_FILL_MODE_SOLID,	//EFM_SOLID,
	};

	static D3D12_CULL_MODE k_CULL_MODE_MAP[ECM_COUNT] =
	{
		D3D12_CULL_MODE_NONE,	//ECM_NONE,
		D3D12_CULL_MODE_FRONT,	//ECM_FRONT,
		D3D12_CULL_MODE_BACK,	//ECM_BACK
	};
	inline void MakeDx12RasterizerDesc(const TPipelineDesc& Desc, D3D12_RASTERIZER_DESC& RasterizerDesc)
	{
		RasterizerDesc.FillMode = k_FILL_MODE_MAP[Desc.RasterizerDesc.FillMode];
		RasterizerDesc.CullMode = k_CULL_MODE_MAP[Desc.RasterizerDesc.CullMode];
		RasterizerDesc.FrontCounterClockwise = FALSE;
		RasterizerDesc.DepthBias = Desc.RasterizerDesc.DepthBias;
		RasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		RasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		RasterizerDesc.DepthClipEnable = TRUE;
		RasterizerDesc.MultisampleEnable = Desc.RasterizerDesc.MultiSampleCount != 0;
		RasterizerDesc.AntialiasedLineEnable = FALSE;
		RasterizerDesc.ForcedSampleCount = 0;
		RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	}

	static D3D12_PRIMITIVE_TOPOLOGY_TYPE k_PRIMITIVE_D3D12_TYPE_MAP[EPT_COUNT] =
	{
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,	//EPT_POINTLIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,	//EPT_LINES,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,	//EPT_LINE_LOOP,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,	//EPT_LINESTRIP,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,	//EPT_TRIANGLELIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,	//EPT_TRIANGLESTRIP,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,	//EPT_TRIANGLE_FAN,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,	//EPT_QUADS,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,	//EPT_QUAD_STRIP,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED	//EPT_POLYGON,
	};

	static D3D_PRIMITIVE_TOPOLOGY k_PRIMITIVE_TYPE_MAP[EPT_COUNT] = 
	{
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,	//EPT_POINTLIST,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,	//EPT_LINES,
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,	//EPT_LINE_LOOP,
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,	//EPT_LINESTRIP,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,	//EPT_TRIANGLELIST,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,	//EPT_TRIANGLESTRIP,
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,	//EPT_TRIANGLE_FAN,
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,	//EPT_QUADS,
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,	//EPT_QUAD_STRIP,
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED	//EPT_POLYGON,
	};
}
#endif	// COMPILE_WITH_RHI_DX12