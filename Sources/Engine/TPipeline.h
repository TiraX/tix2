/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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
		EPSO_STENCIL = 1 << 3
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

	struct TPipelineDesc
	{
		uint32 Flags;
		TBlendState BlendState;
		TRasterizerDesc RasterizerDesc;
		TDepthStencilDesc DepthStencilDesc;
		uint32 VsFormat;
		uint32 InsFormat;
		uint32 PrimitiveType;

		TShaderPtr Shader;

		int32 RTCount;
		E_PIXEL_FORMAT RTFormats[ERTC_COUNT];
		E_PIXEL_FORMAT DepthFormat;
        E_PIXEL_FORMAT StencilFormat;   // For metal

		TPipelineDesc()
			: Flags(EPSO_DEPTH | EPSO_DEPTH_TEST)
			, VsFormat(EVSSEG_POSITION)
			, InsFormat(0)
			, PrimitiveType(EPT_TRIANGLELIST)
			, RTCount(1)
		{
			for (int32 i = 0; i < ERTC_COUNT; i++)
			{
				RTFormats[i] = EPF_UNKNOWN;
			}
			DepthFormat = EPF_UNKNOWN;
			StencilFormat = EPF_UNKNOWN;
		}

		void Enable(E_PIPELINE_STATES_OPTION Option)
		{
			Flags |= Option;
		}

		void Disable(E_PIPELINE_STATES_OPTION Option)
		{
			Flags &= ~Option;
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

		void SetShader(TShaderPtr Shader);

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		FPipelinePtr PipelineResource;

		const TPipelineDesc& GetDesc() const
		{
			return Desc;
		}
	protected:

	protected:
		TPipelineDesc Desc;
	};
    
    ////////////////////////////////////////////////////////////////////////
    
    // For metal: Tile shader pipeline desc corresponding to MTLTileRenderPipelineDescriptor
    class TTilePipeline : public TResource
    {
    public:
        TTilePipeline();
        virtual ~TTilePipeline();
        
        virtual void InitRenderThreadResource() override {};
        virtual void DestroyRenderThreadResource() override {};
        
        void SetRTFormat(uint32 Index, E_PIXEL_FORMAT InFormat)
        {
            TI_ASSERT(Index < ERTC_COUNT);
            RTFormats[Index] = InFormat;
            if (RTCount <= Index)
            {
                RTCount = Index + 1;
            }
        }
        
        void SetSampleCount(uint32 InCount)
        {
            SampleCount = InCount;
        }
        
        void SetThreadGroupSizeMatchesTileSize(bool bMatch)
        {
            ThreadGroupSizeMatchesTileSize = bMatch ? 1 : 0;
        }
        
        E_PIXEL_FORMAT GetRTFormat(uint32 Index) const
        {
            TI_ASSERT(Index < RTCount);
            return RTFormats[Index];
        }
        
        uint32 GetRTCount() const
        {
            return RTCount;
        }
        
        uint32 GetSampleCount() const
        {
            return SampleCount;
        }
        
        uint32 GetThreadGroupSizeMatchesTileSize() const
        {
            return ThreadGroupSizeMatchesTileSize;
        }
        
    protected:
        
    protected:
        E_PIXEL_FORMAT RTFormats[ERTC_COUNT];
        uint32 RTCount;
        uint32 SampleCount;
        uint32 ThreadGroupSizeMatchesTileSize;
    };
}
