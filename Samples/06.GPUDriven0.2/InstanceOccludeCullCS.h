/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FInstanceOccludeCullCS : public FComputeTask
{
public:
	FInstanceOccludeCullCS();
	virtual ~FInstanceOccludeCullCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI
		);
private:

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
};
typedef TI_INTRUSIVE_PTR(FInstanceOccludeCullCS) FInstanceOccludeCullCSPtr;
