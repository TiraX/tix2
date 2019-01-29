/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SeparableSSS.h"

BEGIN_UNIFORM_BUFFER_STRUCT(FSSSBlurUniformBuffer)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, BlurDir)		// xy is blur direction
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, BlurParam)	// x = sssWidth; y = sssFov; z = maxOffsetMm
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(FFloat4, Kernel, [SeparableSSS::SampleCount])
END_UNIFORM_BUFFER_STRUCT(FSSSBlurUniformBuffer)

BEGIN_UNIFORM_BUFFER_STRUCT(FSSSBloomUniformBuffer)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, BloomParam)
END_UNIFORM_BUFFER_STRUCT(FSSSBloomUniformBuffer)


class FS4TempRenderer : public FDefaultRenderer
{
public:
    FS4TempRenderer();
    virtual ~FS4TempRenderer();
    
    virtual void InitInRenderThread() override;
    virtual void Render(FRHI* RHI, FScene* Scene) override;
};

class FSSSSRenderer : public FDefaultRenderer
{
public:
	FSSSSRenderer();
	virtual ~FSSSSRenderer();

	virtual void InitInRenderThread() override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
protected:
	FFullScreenRender FSRender;

	FRenderTargetPtr RT_BasePass;

	// SSS Blur
	FPipelinePtr PL_SSSBlur;
	FRenderTargetPtr RT_SSSBlurX;
	FRenderTargetPtr RT_SSSBlurY;
	FRenderResourceTablePtr TT_SSSBlurX;
	FRenderResourceTablePtr TT_SSSBlurY;
	FSSSBlurUniformBufferPtr UB_SSSBlurX;
	FSSSBlurUniformBufferPtr UB_SSSBlurY;

	// Combine Specular
	FPipelinePtr PL_AddSpecular;
	FArgumentBufferPtr AB_AddSpecular;

	// Bloom
	FRenderTargetPtr RT_GlareDetection;
	FPipelinePtr PL_GlareDetection;
	FArgumentBufferPtr AB_GlareDetection;

	static const int32 BloomPasses = 2;
	struct FBloomPass
	{
		FRenderTargetPtr RT;
		FArgumentBufferPtr AB;
	};
	FPipelinePtr PL_Bloom;
	FBloomPass BloomPass[BloomPasses][2];

	// Combine Bloom and tonemap
	FPipelinePtr PL_Combine;
	FRenderTargetPtr RT_Combine;
	FArgumentBufferPtr AB_Combine;

	FArgumentBufferPtr AB_Result;
	SeparableSSS* S4Effect;
};
