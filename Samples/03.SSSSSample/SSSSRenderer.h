/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FSSSBlurUniformBuffer)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, BlurDir)		// xy is blur direction
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, BlurParam)	// x = sssWidth; y = sssFov; z = maxOffsetMm
END_UNIFORM_BUFFER_STRUCT(FSSSBlurUniformBuffer)

class SeparableSSS;
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

	TMaterialPtr M_SSSBlur;
	FRenderTargetPtr RT_SSSBlurX;
	FRenderTargetPtr RT_SSSBlurY;
	FRenderResourceTablePtr TT_SSSBlurX;
	FRenderResourceTablePtr TT_SSSBlurY;
	FSSSBlurUniformBufferPtr UB_SSSBlurX;
	FSSSBlurUniformBufferPtr UB_SSSBlurY;

	SeparableSSS* S4Effect;
};