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

	void PrepareResources(FRHI * RHI);
	virtual void Run(FRHI * RHI) override;

	void UpdataComputeParams(
		FRHI * RHI,
		const vector2di& InRTSize,
		const vector3df& InViewDir,
		const FMatrix& InViewProjection,
		const SViewFrustum& InFrustum,
		FMeshBufferPtr InSceneMeshBuffer,
		FInstanceBufferPtr InInstanceData, 
		FGPUCommandBufferPtr InCommandBuffer,
		FTexturePtr InHiZTexture,
		FUniformBufferPtr InVisibleClusters
		);


private:
	enum
	{
		SRV_VERTEX_DATA,
		SRV_INDEX_DATA,
		SRV_INSTANCE_DATA,
		SRV_DRAW_COMMANDS,
		SRV_HIZ_TEXTURE,
		SRV_VISIBLE_CLUSTERS,

		UAV_VISIBLE_TRIANGLE_INDICES,
		UAV_OUTPUT_COMMANDS,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	// Dispatch command signature
	FGPUCommandSignaturePtr TriangleCullingCSig;
	FGPUCommandBufferPtr TriangleCullingCB;

	// Compute params
	FCullUniformPtr FrustumUniform;	// b0

	FMeshBufferPtr MeshData;	// t0 t1
	FInstanceBufferPtr InstanceData;	// t2
	FGPUCommandBufferPtr DrawCommandBuffer;	// t3
	FTexturePtr HiZTexture;	// t4
	FUniformBufferPtr VisibleClusters;	// t5

	FUniformBufferPtr VisibleTriangleIndices;	// u0
	FGPUCommandBufferPtr OutDrawCommands;	// u1

	FUniformBufferPtr ResetCounterBuffer;
	FGPUCommandBufferPtr EmptyCommandBuffer;

};
typedef TI_INTRUSIVE_PTR(FTriangleCullCS) FTriangleCullCSPtr;