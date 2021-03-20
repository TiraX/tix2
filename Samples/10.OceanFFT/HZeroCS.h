/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

class FHZeroCS : public FComputeTask
{
public:
	FHZeroCS();
	virtual ~FHZeroCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI
	);

private:
	enum
	{
		UAV_AO_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:
};
typedef TI_INTRUSIVE_PTR(FHZeroCS) FHZeroCSPtr;
