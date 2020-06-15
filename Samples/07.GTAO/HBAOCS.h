/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FHBAOUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, ScreenSize)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, InvFocalLen)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, ViewDir)
END_UNIFORM_BUFFER_STRUCT(FHBAOUniform)

class FHBAOCS : public FComputeTask
{
public:
	FHBAOCS();
	virtual ~FHBAOCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		const vector3df& ViewDir,
		FTexturePtr InSceneTexture,
		FTexturePtr InSceneDepth
		);

private:
	enum
	{
		SRV_SCENE_NORMAL,
		SRV_SCENE_DEPTH,

		UAV_AO_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:
	FHBAOUniformPtr InfoUniform;

	FRenderResourceTablePtr ResourceTable;
	
	FTexturePtr SceneNormal;
	FTexturePtr SceneDepth;

	FTexturePtr AOTexture;

};
typedef TI_INTRUSIVE_PTR(FHBAOCS) FHBAOCSCSPtr;
