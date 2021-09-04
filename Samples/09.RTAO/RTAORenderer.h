/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FPathtracerUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, CamPos)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, ProjectionToWorld)
END_UNIFORM_BUFFER_STRUCT(FPathtracerUniform)

class FRTAORenderer : public FDefaultRenderer
{
public:
	FRTAORenderer();
	virtual ~FRTAORenderer();

	static FRTAORenderer* Get();

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
	void UpdateCamInfo(FScene* Scene);
	void DrawSceneTiles(FRHI* RHI, FScene * Scene);

private:
	enum
	{
		GBUFFER_COLOR,

		GBUFFER_COUNT,
	};

	FArgumentBufferPtr AB_Result;

	FFullScreenRender FSRender;
	FRenderTargetPtr RT_BasePass;

	FRtxPipelinePtr RtxPSO;
	FTexturePtr T_GBuffer[GBUFFER_COUNT];

	FPathtracerUniformPtr UB_Pathtracer;

	FRenderResourceTablePtr ResourceTable;
};
