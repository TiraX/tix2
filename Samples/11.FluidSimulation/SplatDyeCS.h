/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SplatParam.h"

class FSplatDyeCS : public FComputeTask
{
public:
	FSplatDyeCS();
	virtual ~FSplatDyeCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdateComputeParams(
		FRHI* RHI,
		FUniformBufferPtr InParam,
		const vector2df& MousePointUV,
		const vector2df& MouseMoveDir,
		const float RadiusScale,
		FTexturePtr InDye,
		FTexturePtr OutDye
	);

private:
	enum
	{
		SRV_DYE,
		UAV_DYE,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_Fluid2dParam;
	FSplatParamPtr UB_SplatParam;

};
typedef TI_INTRUSIVE_PTR(FSplatDyeCS) FSplatDyeCSPtr;
