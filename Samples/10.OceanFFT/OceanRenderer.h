/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "GaussRandomCS.h"
#include "HZeroCS.h"
#include "HKtCS.h"
#include "IFFTCS.h"
#include "DisplacementCS.h"
#include "GenerateNormalCS.h"

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
	void CreateButterFlyTexture();

	void DrawOceanGrid(FRHI* RHI, FScene* Scene);

private:
	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FTexturePtr GaussRandomTexture;
	FTexturePtr ButterFlyTexture;

	// Ocean Compute Shader
	FHZeroCSPtr HZeroCS;
	FHKtCSPtr HKtCS;
	FIFFTCSPtr IFFTHeightCS[3];
	FIFFTCSPtr IFFTJacobCS[3];
	FDisplacementCSPtr DisplacementCS;
	FGenerateNormalCSPtr GenerateNormalCS;

	// Ocean grid mesh
	FPrimitivePtr OceanPrimitive;
	FRenderResourceTablePtr OceanDisplacementResource;
	FRenderResourceTablePtr OceanNormalResource;
};
