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
private:

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
};
typedef TI_INTRUSIVE_PTR(FInstanceFrustumCullCS) FInstanceFrustumCullCSPtr;
