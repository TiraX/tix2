/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FHBAOCS : public FComputeTask
{
public:
	FHBAOCS();
	virtual ~FHBAOCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI
		);

private:
	enum
	{
		SRV_CLUSTER_BOUNDING_DATA,
		SRV_INSTANCE_DATA,
		SRV_DRAW_COMMANDS,
		SRV_HIZ_TEXTURE,
		SRV_COLLECTED_CLUSTERS,

		UAV_VISIBLE_CLUSTERS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;


};
typedef TI_INTRUSIVE_PTR(FHBAOCS) FHBAOCSCSPtr;
