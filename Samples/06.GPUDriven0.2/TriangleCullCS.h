/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once
#include "ComputeUniforms.h"

class FTriangleCullCS : public FComputeTask
{
public:
	FTriangleCullCS();
	virtual ~FTriangleCullCS();

	void PrepareResources(FRHI * RHI, uint32 InMaxClustersInScene);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		const vector2di& InRTSize,
		const vector3df& InViewDir,
		const FMatrix& InViewProjection,
		const SViewFrustum& InFrustum,
		FInstanceBufferPtr InInstanceData, 
		FGPUCommandBufferPtr InCommandBuffer,
		FTexturePtr InHiZTexture
		);


private:
	enum
	{
		CBV_VIEW_INFO,

		SRV_INSTANCE_DATA,
		SRV_DRAW_COMMANDS,
		SRV_HIZ_TEXTURE,

		UAV_VISIBLE_CLUSTERS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Dispatch command signature
	FGPUCommandSignaturePtr TriangleCullingCSig;
	FGPUCommandBufferPtr TriangleCullingCB;

	// Compute params
	// Root Constant b0 in command buffer

	FCullUniformPtr FrustumUniform;	// b1

	FInstanceBufferPtr InstanceData;	// t0
	FGPUCommandBufferPtr DrawCommandBuffer;	// t1
	FTexturePtr HiZTexture;	// t2

	FUniformBufferPtr VisibleClusters;	// u0

	FUniformBufferPtr ResetCounterBuffer;

};
typedef TI_INTRUSIVE_PTR(FTriangleCullCS) FTriangleCullCSPtr;
