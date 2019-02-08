/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_METAL

namespace tix
{
	inline MTLPixelFormat GetMetalPixelFormat(E_PIXEL_FORMAT InFormat)
	{
		switch (InFormat)
		{
		case EPF_A8:
			return MTLPixelFormatR8Unorm;
		case EPF_RGBA8:
			return MTLPixelFormatRGBA8Unorm;
		case EPF_RGBA8_SRGB:
			return MTLPixelFormatRGBA8Unorm_sRGB;
		case EPF_BGRA8:
			return MTLPixelFormatBGRA8Unorm;
		case EPF_BGRA8_SRGB:
			return MTLPixelFormatBGRA8Unorm_sRGB;
		case EPF_R16F:
			return MTLPixelFormatR16Float;
		case EPF_RG16F:
			return MTLPixelFormatRG16Float;
		case EPF_RGBA16F:
			return MTLPixelFormatRGBA16Float;
		case EPF_R32F:
			return MTLPixelFormatR32Float;
		case EPF_RG32F:
			return MTLPixelFormatRG32Float;
		//case EPF_RGB32F:
		//	return MTLPixelFormatInvalid;
		case EPF_RGBA32F:
			return MTLPixelFormatRGBA32Float;
		//case EPF_DEPTH16:
		//	return MTLPixelFormatDepth16Unorm;
		case EPF_DEPTH32:
			return MTLPixelFormatDepth32Float;
		case EPF_DEPTH24_STENCIL8:
			return MTLPixelFormatDepth32Float_Stencil8;
        case EPF_STENCIL8:
            return MTLPixelFormatStencil8;
		case EPF_ASTC4x4:
			return MTLPixelFormatASTC_4x4_LDR;
        case EPF_ASTC4x4_SRGB:
            return MTLPixelFormatASTC_4x4_sRGB;
        case EPF_ASTC6x6:
            return MTLPixelFormatASTC_6x6_LDR;
        case EPF_ASTC6x6_SRGB:
            return MTLPixelFormatASTC_6x6_sRGB;
        case EPF_ASTC8x8:
            return MTLPixelFormatASTC_8x8_LDR;
        case EPF_ASTC8x8_SRGB:
            return MTLPixelFormatASTC_8x8_sRGB;
        case EPF_UNKNOWN:
            return MTLPixelFormatInvalid;
        default:
            TI_ASSERT(0);
            return MTLPixelFormatInvalid;
		}
	};

    static const MTLVertexFormat k_MESHBUFFER_STREAM_FORMAT_MAP[ESSI_TOTAL] =
    {
        MTLVertexFormatFloat3,    // ESSI_POSITION,
        MTLVertexFormatUChar4Normalized,        // ESSI_NORMAL,
        MTLVertexFormatUChar4Normalized,    // ESSI_COLOR,
        MTLVertexFormatHalf2,    // ESSI_TEXCOORD0,
        MTLVertexFormatHalf2,    // ESSI_TEXCOORD1,
        MTLVertexFormatUChar4Normalized,    // ESSI_TANGENT,
        MTLVertexFormatUChar4,    // ESSI_BLENDINDEX,
        MTLVertexFormatUChar4Normalized,    // ESSI_BLENDWEIGHT
    };

	static const MTLBlendFactor k_BLEND_FACTOR_MAP[EBF_COUNT] =
	{
		MTLBlendFactorZero,	//EBF_ZERO,
		MTLBlendFactorOne,	//EBF_ONE,
		MTLBlendFactorSourceColor,	//EBF_SRC_COLOR,
		MTLBlendFactorOneMinusSourceColor,	//EBF_ONE_MINUS_SRC_COLOR,
		MTLBlendFactorDestinationColor,	//EBF_DEST_COLOR,
		MTLBlendFactorOneMinusDestinationColor,	//EBF_ONE_MINUS_DEST_COLOR,
		MTLBlendFactorSourceAlpha,	//EBF_SRC_ALPHA,
		MTLBlendFactorOneMinusSourceAlpha,	//EBF_ONE_MINUS_SRC_ALPHA,
		MTLBlendFactorDestinationAlpha,	//EBF_DST_ALPHA,
		MTLBlendFactorOneMinusDestinationAlpha,	//EBF_ONE_MINUS_DST_ALPHA,
		MTLBlendFactorSource1Color,	//EBF_CONSTANT_COLOR,
		MTLBlendFactorOneMinusSource1Color,	//EBF_ONE_MINUS_CONSTANT_COLOR,
		MTLBlendFactorSource1Alpha,	//EBF_CONSTANT_ALPHA,
		MTLBlendFactorOneMinusSource1Alpha,	//EBF_ONE_MINUS_CONSTANT_ALPHA,
		MTLBlendFactorSourceAlphaSaturated,	//EBF_SRC_ALPHA_SATURATE
	};

	static const MTLBlendOperation k_BLEND_OPERATION_MAP[EBE_COUNT] =
	{
		MTLBlendOperationAdd,	//EBE_FUNC_ADD,
		MTLBlendOperationSubtract,	//EBE_FUNC_SUBTRACT,
		MTLBlendOperationReverseSubtract,	//EBE_FUNC_REVERSE_SUBTRACT,
		MTLBlendOperationMin,	//EBE_MIN,
		MTLBlendOperationMax,	//EBE_MAX
	};

//    inline DXGI_FORMAT GetSRGBFormat(DXGI_FORMAT InFormat)
//    {
//        switch (InFormat)
//        {
//        // Keep input format
//        case DXGI_FORMAT_R8_UNORM:
//            break;
//
//        // Convert to SRGB format
//        case DXGI_FORMAT_BC1_UNORM:
//            return DXGI_FORMAT_BC1_UNORM_SRGB;
//        case DXGI_FORMAT_BC2_UNORM:
//            return DXGI_FORMAT_BC2_UNORM_SRGB;
//        case DXGI_FORMAT_BC3_UNORM:
//            return DXGI_FORMAT_BC3_UNORM_SRGB;
//        case DXGI_FORMAT_R8G8B8A8_UNORM:
//            return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
//        default:
//            TI_ASSERT(0);
//            break;
//        }
//        return InFormat;
//    };
//
//    inline void MakeDx12BlendState(const TPipelineDesc& Desc, D3D12_BLEND_DESC& BlendState)
//    {
//        BlendState.AlphaToCoverageEnable = FALSE;
//        BlendState.IndependentBlendEnable = FALSE;
//
//        D3D12_RENDER_TARGET_BLEND_DESC BlendDesc;
//        BlendDesc.BlendEnable = Desc.IsEnabled(EPSO_BLEND);
//        BlendDesc.LogicOpEnable = FALSE;
//        BlendDesc.SrcBlend = k_BLEND_FUNC_MAP[Desc.BlendState.SrcBlend];
//        BlendDesc.DestBlend = k_BLEND_FUNC_MAP[Desc.BlendState.DestBlend];
//        BlendDesc.BlendOp = k_BLEND_EQUATION_MAP[Desc.BlendState.BlendOp];
//        BlendDesc.SrcBlendAlpha = k_BLEND_FUNC_MAP[Desc.BlendState.SrcBlendAlpha];
//        BlendDesc.DestBlendAlpha = k_BLEND_FUNC_MAP[Desc.BlendState.DestBlendAlpha];
//        BlendDesc.BlendOpAlpha = k_BLEND_EQUATION_MAP[Desc.BlendState.BlendOpAlpha];
//        BlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
//        BlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
//        for (int32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
//            BlendState.RenderTarget[i] = BlendDesc;
//    }

    static const MTLCompareFunction k_COMPARE_FUNC_MAP[ECF_COUNT] =
    {
        MTLCompareFunctionNever,    //ECF_NEVER,
        MTLCompareFunctionLess,    //ECF_LESS,
        MTLCompareFunctionLessEqual,    //ECF_LESS_EQUAL,
        MTLCompareFunctionEqual,    //ECF_EQUAL,
        MTLCompareFunctionGreater,    //ECF_GREATER,
        MTLCompareFunctionNotEqual,    //ECF_NOT_EQUAL,
        MTLCompareFunctionGreaterEqual,    //ECF_GREATER_EQUAL,
        MTLCompareFunctionAlways,    //ECF_ALWAYS,
    };
    
    static const MTLStencilOperation k_STENCIL_OP_MAP[ESO_COUNT] =
    {
        MTLStencilOperationKeep,    //ESO_KEEP,
        MTLStencilOperationZero,    //ESO_ZERO,
        MTLStencilOperationReplace,    //ESO_REPLACE,
        MTLStencilOperationIncrementClamp,    //ESO_INCR_SAT,
        MTLStencilOperationDecrementClamp,    //ESO_DECR_SAT,
        MTLStencilOperationInvert,    //ESO_INVERT,
        MTLStencilOperationIncrementWrap,    //ESO_INCR,
        MTLStencilOperationDecrementWrap,    //ESO_DECR,
    };

//    inline void MakeDx12DepthStencilState(const TPipelineDesc& Desc, D3D12_DEPTH_STENCIL_DESC& DepthStencilState)
//    {
//        DepthStencilState.DepthEnable = Desc.IsEnabled(EPSO_DEPTH);
//        DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
//        DepthStencilState.DepthFunc = k_COMPARISON_FUNC_MAP[Desc.DepthStencilDesc.DepthFunc];
//        DepthStencilState.StencilEnable = Desc.IsEnabled(EPSO_STENCIL);
//        DepthStencilState.StencilReadMask = Desc.DepthStencilDesc.StencilReadMask;
//        DepthStencilState.StencilWriteMask = Desc.DepthStencilDesc.StencilWriteMask;
//
//        DepthStencilState.FrontFace.StencilFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilFailOp];
//        DepthStencilState.FrontFace.StencilDepthFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilDepthFailOp];
//        DepthStencilState.FrontFace.StencilPassOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.FrontFace.StencilPassOp];
//        DepthStencilState.FrontFace.StencilFunc = k_COMPARISON_FUNC_MAP[Desc.DepthStencilDesc.FrontFace.StencilFunc];
//
//        DepthStencilState.BackFace.StencilFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilFailOp];
//        DepthStencilState.BackFace.StencilDepthFailOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilDepthFailOp];
//        DepthStencilState.BackFace.StencilPassOp = k_STENCIL_OP_MAP[Desc.DepthStencilDesc.BackFace.StencilPassOp];
//        DepthStencilState.BackFace.StencilFunc = k_COMPARISON_FUNC_MAP[Desc.DepthStencilDesc.BackFace.StencilFunc];
//    }
//
//    static const D3D12_FILL_MODE k_FILL_MODE_MAP[EFM_COUNT] =
//    {
//        D3D12_FILL_MODE_WIREFRAME,    //EFM_WIREFRAME,
//        D3D12_FILL_MODE_SOLID,    //EFM_SOLID,
//    };
//
    static const MTLCullMode k_CULL_MODE_MAP[ECM_COUNT] =
    {
        MTLCullModeNone,    //ECM_NONE,
        MTLCullModeFront,    //ECM_FRONT,
        MTLCullModeBack,    //ECM_BACK
    };
//     */
//
//    inline void MakeMetalRasterizerDesc(const TPipelineDesc& Desc, MTLRenderPipelineDescriptor* PipelineDescriptor)
//    {
//        RasterizerDesc.FillMode = k_FILL_MODE_MAP[Desc.RasterizerDesc.FillMode];
//        RasterizerDesc.CullMode = k_CULL_MODE_MAP[Desc.RasterizerDesc.CullMode];
//        RasterizerDesc.FrontCounterClockwise = FALSE;
//        RasterizerDesc.DepthBias = Desc.RasterizerDesc.DepthBias;
//        RasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
//        RasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
//        RasterizerDesc.DepthClipEnable = TRUE;
//        RasterizerDesc.MultisampleEnable = Desc.RasterizerDesc.MultiSampleCount != 0;
//        RasterizerDesc.AntialiasedLineEnable = FALSE;
//        RasterizerDesc.ForcedSampleCount = 0;
//        RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
//    }
//
//    static const D3D12_PRIMITIVE_TOPOLOGY_TYPE k_PRIMITIVE_D3D12_TYPE_MAP[EPT_COUNT] =
//    {
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,    //EPT_POINTLIST,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,    //EPT_LINES,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,    //EPT_LINE_LOOP,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,    //EPT_LINESTRIP,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,    //EPT_TRIANGLELIST,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,    //EPT_TRIANGLESTRIP,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,    //EPT_TRIANGLE_FAN,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,    //EPT_QUADS,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,    //EPT_QUAD_STRIP,
//        D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED    //EPT_POLYGON,
//    };
//
    
    static const MTLPrimitiveType k_PRIMITIVE_TYPE_MAP[EPT_COUNT] =
    {
        MTLPrimitiveTypePoint,    //EPT_POINTLIST,
        MTLPrimitiveTypeLine,    //EPT_LINES,
        MTLPrimitiveTypeLineStrip,    //EPT_LINESTRIP,
        MTLPrimitiveTypeTriangle,    //EPT_TRIANGLELIST,
        MTLPrimitiveTypeTriangleStrip,    //EPT_TRIANGLESTRIP,
    };
    
    static const MTLIndexType k_INDEX_TYPE_MAP[EIT_COUNT] =
    {
        MTLIndexTypeUInt16, //EIT_16BIT,
        MTLIndexTypeUInt32, //EIT_32BIT,
    };
//
//    static const E_RENDER_RESOURCE_HEAP_TYPE Dx2TiXHeapMap[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] =
//    {
//        EHT_UNIFORMBUFFER,    //D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV = 0,
//        EHT_SAMPLER,        //D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER = (D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV + 1) ,
//        EHT_RENDERTARGET,    //D3D12_DESCRIPTOR_HEAP_TYPE_RTV = (D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER + 1) ,
//        EHT_DEPTHSTENCIL,    //D3D12_DESCRIPTOR_HEAP_TYPE_DSV = (D3D12_DESCRIPTOR_HEAP_TYPE_RTV + 1) ,
//    };
//
//    inline E_RENDER_RESOURCE_HEAP_TYPE GetTiXHeapTypeFromDxHeap(D3D12_DESCRIPTOR_HEAP_TYPE DxHeap)
//    {
//        return Dx2TiXHeapMap[DxHeap];
//    }
//
//    static const D3D12_DESCRIPTOR_HEAP_TYPE TiX2DxHeapMap[EHT_COUNT] =
//    {
//        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,            //EHT_RENDERTARGET = 0,
//        D3D12_DESCRIPTOR_HEAP_TYPE_DSV,            //EHT_DEPTHSTENCIL,
//        D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,        //EHT_SAMPLER,
//        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,    //EHT_UNIFORMBUFFER,
//        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,    //EHT_TEXTURE,
//    };
//
//    inline D3D12_DESCRIPTOR_HEAP_TYPE GetDxHeapTypeFromTiXHeap(E_RENDER_RESOURCE_HEAP_TYPE TiXHeap)
//    {
//        return TiX2DxHeapMap[TiXHeap];
//    }
//
//    static const D3D12_FILTER TiX2DxTextureFilterMap[ETFT_COUNT] =
//    {
//        D3D12_FILTER_MIN_MAG_MIP_POINT,    //ETFT_MINMAG_NEAREST_MIP_NEAREST = 0,
//        D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT,    //ETFT_MINMAG_LINEAR_MIP_NEAREST,
//        D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR,    //ETFT_MINMAG_NEAREST_MIPMAP_LINEAR,
//        D3D12_FILTER_MIN_MAG_MIP_LINEAR,    //ETFT_MINMAG_LINEAR_MIPMAP_LINEAR,
//    };
//
//    inline D3D12_FILTER GetDxTextureFilterFromTiX(E_TEXTURE_FILTER_TYPE Filter)
//    {
//        return TiX2DxTextureFilterMap[Filter];
//    }
//
//    static const D3D12_TEXTURE_ADDRESS_MODE TiX2DxTextureAddressMode[ETC_COUNT] =
//    {
//        D3D12_TEXTURE_ADDRESS_MODE_WRAP,    //ETC_REPEAT = 0,
//        D3D12_TEXTURE_ADDRESS_MODE_CLAMP,    //ETC_CLAMP_TO_EDGE,
//        D3D12_TEXTURE_ADDRESS_MODE_MIRROR,    //ETC_MIRROR,
//    };
//
//    inline D3D12_TEXTURE_ADDRESS_MODE GetDxTextureAddressModeFromTiX(E_TEXTURE_ADDRESS_MODE AddressMode)
//    {
//        return TiX2DxTextureAddressMode[AddressMode];
//    }
//
//    static const D3D12_SHADER_VISIBILITY TiX2DxShaderStage[ESS_COUNT] =
//    {
//        D3D12_SHADER_VISIBILITY_VERTEX,    //ESS_VERTEX_SHADER,
//        D3D12_SHADER_VISIBILITY_PIXEL,    //ESS_PIXEL_SHADER,
//        D3D12_SHADER_VISIBILITY_DOMAIN,    //ESS_DOMAIN_SHADER,
//        D3D12_SHADER_VISIBILITY_HULL,    //ESS_HULL_SHADER,
//        D3D12_SHADER_VISIBILITY_GEOMETRY,    //ESS_GEOMETRY_SHADER,
//    };
//
//    inline D3D12_SHADER_VISIBILITY GetDxShaderVisibilityFromTiX(E_SHADER_STAGE ShaderStage)
//    {
//        return TiX2DxShaderStage[ShaderStage];
//    }
    
    static const MTLLoadAction k_LOAD_ACTION_MAP[ERT_LOAD_ACTION_NUM] =
    {
        MTLLoadActionDontCare,    //ERT_LOAD_DONTCARE,
        MTLLoadActionLoad,    //ERT_LOAD_LOAD,
        MTLLoadActionClear,    //ERT_LOAD_CLEAR,
    };
    
    static const MTLStoreAction k_STORE_ACTION_MAP[ERT_STORE_ACTION_NUM] =
    {
        MTLStoreActionDontCare, //ERT_STORE_DONTCARE,
        MTLStoreActionStore,    //ERT_STORE_STORE,
        MTLStoreActionMultisampleResolve,   //ERT_STORE_MULTISAMPLE_RESOLVE,
        MTLStoreActionStoreAndMultisampleResolve,   //ERT_STORE_STORE_AND_MULTISAMPLE_RESOLVE,
    };
}
#endif	// COMPILE_WITH_RHI_METAL
