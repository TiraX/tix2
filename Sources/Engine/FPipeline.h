/*
TiX Engine v2.0 Copyright (C) 2018~2019
By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class FPipeline : public FRenderResource
	{
	public:
		FPipeline(FShaderPtr InShader);
		virtual ~FPipeline();

		FShaderPtr GetShader()
		{
			return Shader;
		}
	protected:

	protected:
		FShaderPtr Shader;
	};

	///////////////////////////////////////////////////////////////////////////

	//struct FRenderPipelineDesc
	//{
	//	uint32 Flags;
	//	TBlendState BlendState;
	//	TRasterizerDesc RasterizerDesc;
	//	TDepthStencilDesc DepthStencilDesc;
	//	uint32 VsFormat;
	//	uint32 PrimitiveType;

	//	int32 RTCount;
	//	E_PIXEL_FORMAT RTFormats[ERTC_COUNT];
	//	E_PIXEL_FORMAT DepthFormat;
	//	E_PIXEL_FORMAT StencilFormat;   // For metal

	//	FRenderPipelineDesc()
	//		: Flags(EPSO_DEPTH | EPSO_DEPTH_TEST)
	//		, VsFormat(EVSSEG_POSITION)
	//		, PrimitiveType(EPT_TRIANGLELIST)
	//		, RTCount(1)
	//	{
	//		for (int32 i = 0; i < ERTC_COUNT; i++)
	//		{
	//			RTFormats[i] = EPF_UNKNOWN;
	//		}
	//		DepthFormat = EPF_UNKNOWN;
	//		StencilFormat = EPF_UNKNOWN;
	//	}

	//	bool IsEnabled(E_PIPELINE_STATES_OPTION Option) const
	//	{
	//		return (Flags & Option) != 0;
	//	}

	//	FRenderPipelineDesc& operator = (const TPipelineDesc& Other)
	//	{
	//		Flags = Other.Flags;
	//		BlendState = Other.BlendState;
	//		RasterizerDesc = Other.RasterizerDesc;
	//		DepthStencilDesc = Other.DepthStencilDesc;
	//		VsFormat = Other.VsFormat;
	//		PrimitiveType = Other.PrimitiveType;

	//		RTCount = Other.RTCount;
	//		for (int32 i = 0; i < ERTC_COUNT; i++)
	//		{
	//			RTFormats[i] = Other.RTFormats[i];
	//		}
	//		DepthFormat = Other.DepthFormat;
	//		StencilFormat = Other.StencilFormat;

	//		return *this;
	//	}
	//};

	//class FRenderPipeline : public FPipeline
	//{
	//public:
	//	FRenderPipeline(FShaderPtr InShader);
	//	virtual ~FRenderPipeline();

	//	FRenderPipelineDesc& GetDesc()
	//	{
	//		return Desc;
	//	}

	//	void SetDesc(const TPipelineDesc& TDesc)
	//	{
	//		Desc = TDesc;
	//	}

	//protected:
	//	FRenderPipelineDesc Desc;
	//};

	///////////////////////////////////////////////////////////////////////////

	//class FComputePipeline : public FPipeline
	//{
	//public:
	//	FComputePipeline(FShaderPtr InShader);
	//	virtual ~FComputePipeline();

	//protected:
	//};
}
