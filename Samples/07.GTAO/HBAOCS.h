/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 
#define MAX_DIR 16
BEGIN_UNIFORM_BUFFER_STRUCT(FHBAOUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, ScreenSize)	// xy = Size; zw = InvSize;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, FocalLen)		// xy = FocalLen; zw = InvFocalLen;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Radius)		// x = radius; y = radius^2; z = 1.0/radius
END_UNIFORM_BUFFER_STRUCT(FHBAOUniform)

class FHBAOCS : public FComputeTask
{
public:
	FHBAOCS();
	virtual ~FHBAOCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		float Fov,
		FTexturePtr InSceneTexture,
		FTexturePtr InSceneDepth
	);

	FTexturePtr GetAOTexture()
	{
		return AOTexture;
	}

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

	float FoV;

	FTexturePtr AOTexture;

};
typedef TI_INTRUSIVE_PTR(FHBAOCS) FHBAOCSCSPtr;
