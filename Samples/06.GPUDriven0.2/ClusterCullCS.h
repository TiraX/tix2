/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FClusterCullCS : public FComputeTask
{
public:
	FClusterCullCS();
	virtual ~FClusterCullCS();

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI
		);


private:
	enum
	{
		SRV_PRIMITIVE_BBOXES,
		SRV_INSTANCE_METAINFO,
		SRV_INSTANCE_DATA,
		SRV_DRAW_COMMAND_BUFFER,
		SRV_VISIBLE_INSTANCE_INDEX,
		SRV_VISIBLE_INSTANCE_COUNT,
		SRV_HIZ_TEXTURE,

		UAV_COMPACT_INSTANCE_DATA,
		UAV_CULLED_DRAW_COMMAND_BUFFER,
		UAV_COLLECTED_CLUSTERS_COUNT,
		UAV_COLLECTED_CLUSTERS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

};
typedef TI_INTRUSIVE_PTR(FClusterCullCS) FClusterCullCSPtr;
