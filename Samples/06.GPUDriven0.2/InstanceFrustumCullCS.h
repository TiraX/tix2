/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FInstanceFrustumCullCS : public FComputeTask
{
public:
	FInstanceFrustumCullCS();
	virtual ~FInstanceFrustumCullCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

private:

private:
};
typedef TI_INTRUSIVE_PTR(FInstanceFrustumCullCS) FInstanceFrustumCullCSPtr;
