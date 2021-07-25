/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "SplatParam.h"

class FSplatVelCS : public FComputeTask
{
public:
	FSplatVelCS();
	virtual ~FSplatVelCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdateComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InParam,
		const vector2df& MousePointUV,
		const vector2df& MouseMoveDir,
		const float RadiusScale,
		FTexturePtr InVelocity,
		FTexturePtr OutVelocity
		);

private:
	enum
	{
		SRV_VELOCITY,
		UAV_VELOCITY,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	// CBV
	FUniformBufferPtr UBRef_Fluid2dParam;
	FSplatParamPtr UB_SplatParam;

};
typedef TI_INTRUSIVE_PTR(FSplatVelCS) FSplatVelCSPtr;
