/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "GaussRandomCS.h"
#include "HZeroCS.h"

class FOceanRenderer : public FDefaultRenderer
{
public:
	static const int32 FFT_Size = 512;

	FOceanRenderer();
	virtual ~FOceanRenderer();

	static FOceanRenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	void CreateGaussRandomTexture();

private:
	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FTexturePtr GaussRandomTexture;
	// Ocean Compute Shader
	//FGaussRandomCSPtr GaussRandomCS;	// Create this GaussRandom on CPU
	FHZeroCSPtr HZeroCS;;
};
