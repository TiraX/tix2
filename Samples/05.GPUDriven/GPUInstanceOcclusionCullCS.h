/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FOcclusionInfo)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, ViewProjection)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, RTSize)	// xy = Screen RT Size, z = Max Mips
END_UNIFORM_BUFFER_STRUCT(FOcclusionInfo)

class FGPUInstanceOcclusionCullCS : public FComputeTask
{
public:
	FGPUInstanceOcclusionCullCS();
	virtual ~FGPUInstanceOcclusionCullCS();

	void PrepareResources(FRHI * RHI, const vector2di& RTSize, FTexturePtr HiZTexture);
	void UpdateComputeArguments(
		FRHI * RHI,
		const FMatrix& ViewProjection,
		FUniformBufferPtr PrimitiveBBoxes,
		FUniformBufferPtr InstanceMetaInfo,
		FInstanceBufferPtr SceneInstanceData,
		FUniformBufferPtr FrustumCullResult,
		uint32 InstancesCountIntersectWithFrustum);
	virtual void Run(FRHI * RHI) override;

	FUniformBufferPtr GetVisibleResult()
	{
		return VisibilityResult;
	}
private:

private:
	FOcclusionInfoPtr OcclusionInfo;
	FRenderResourceTablePtr ResourceTable;
	FUniformBufferPtr FrustumCullResult;
	FUniformBufferPtr VisibilityResult;
	uint32 InstancesNeedToCull;
};
typedef TI_INTRUSIVE_PTR(FGPUInstanceOcclusionCullCS) FGPUInstanceOcclusionCullCSPtr;
