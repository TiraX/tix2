/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

class FDistantSkyLightLutCS : public FComputeTask
{
public:
	FDistantSkyLightLutCS();
	virtual ~FDistantSkyLightLutCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI
	);

private:
	enum
	{
		SRV_SCENE_NORMAL,
		SRV_SCENE_DEPTH,
		SRV_RANDOM_TEXTURE,

		UAV_AO_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:

};
typedef TI_INTRUSIVE_PTR(FDistantSkyLightLutCS) FDistantSkyLightLutCSPtr;
