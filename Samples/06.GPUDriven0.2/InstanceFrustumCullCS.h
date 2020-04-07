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

	void UpdataComputeParams(
		FRHI * RHI,
		FUniformBufferPtr InFrustumUniform,
		FUniformBufferPtr InPrimitiveBBoxes,
		FUniformBufferPtr InInstanceMetaInfo,
		FInstanceBufferPtr InInstanceData,
		FGPUCommandSignaturePtr InCommandSignature,
		FGPUCommandBufferPtr InDrawCommandBuffer
	);

	FGPUCommandBufferPtr GetCulledDrawCommandBuffer()
	{
		return CulledDrawCommandBuffer;
	}

	FInstanceBufferPtr GetCompactInstanceBuffer()
	{
		return CompactInstanceData;
	}

	FUniformBufferPtr GetVisibleInstanceIndex()
	{
		return VisibleInstanceIndex;
	}

	FUniformBufferPtr GetVisibleInstanceCount()
	{
		return VisibleInstanceCount;
	}
private:
	enum 
	{
		PARAM_PRIMITIVE_BBOXES,
		PARAM_INSTANCE_METAINFO,
		PARAM_INSTANCE_DATA,
		PARAM_DRAW_COMMAND_BUFFER,

		PARAM_COMPACT_INSTANCE_DATA,
		PARAM_CULLED_DRAW_COMMAND_BUFFER,
		PARAM_VISIBLE_INSTANCE_INDEX,
		PARAM_VISIBLE_INSTANCE_COUNT,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Compute params
	FUniformBufferPtr FrustumUniform;	// b0

	FUniformBufferPtr PrimitiveBBoxes;	// t0
	FUniformBufferPtr InstanceMetaInfo;	// t1
	FInstanceBufferPtr InstanceData;	// t2
	FGPUCommandBufferPtr DrawCommandBuffer;	// t3

	FInstanceBufferPtr CompactInstanceData;	// u0
	FGPUCommandBufferPtr CulledDrawCommandBuffer;	// u1
	FUniformBufferPtr VisibleInstanceIndex;	// u2
	FUniformBufferPtr VisibleInstanceCount;	// u3

	FUniformBufferPtr ResetCommandBuffer;
};
typedef TI_INTRUSIVE_PTR(FInstanceFrustumCullCS) FInstanceFrustumCullCSPtr;
