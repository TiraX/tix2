/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FPathtracerUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, CamPos)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, CamU)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, CamV)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, CamW)
END_UNIFORM_BUFFER_STRUCT(FPathtracerUniform)

class FRTAORenderer : public FDefaultRenderer
{
public:
	FRTAORenderer();
	virtual ~FRTAORenderer();

	static FRTAORenderer* Get();

	void UpdateCamInfo(const vector3df& Pos, const vector3df& Dir, const vector3df& Hor, const vector3df& Ver);

	virtual void InitInRenderThread() override;
	virtual void InitRenderFrame(FScene* Scene) override;
	virtual void Render(FRHI* RHI, FScene* Scene) override;

private:
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
