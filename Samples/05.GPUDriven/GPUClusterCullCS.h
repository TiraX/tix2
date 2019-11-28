/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FCullUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, ViewDir)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FMatrix, ViewProjection)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, RTSize)	// xy = Screen RT Size, z = Max Mips
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER_ARRAY(FFloat4, Planes, [6])
END_UNIFORM_BUFFER_STRUCT(FCullUniform)

class FGPUClusterCullCS : public FComputeTask
{
public:
	FGPUClusterCullCS();
	virtual ~FGPUClusterCullCS();

	void PrepareResources(FRHI * RHI, const vector2di& RTSize, FTexturePtr HiZTexture, FUniformBufferPtr VisibleClusters);
	void UpdateComputeArguments(
		FRHI * RHI,
		const vector3df& ViewDir,
		const FMatrix& ViewProjection,
		const SViewFrustum& InFrustum,
		FUniformBufferPtr ClusterMetaData,
		FInstanceBufferPtr SceneInstanceData);
	virtual void Run(FRHI * RHI) override;

	FGPUCommandBufferPtr GetDispatchCommandBuffer()
	{
		return GPUCommandBuffer;
	}
private:

private:
	FCullUniformPtr CullUniform;
	FRenderResourceTablePtr ResourceTable;

	FCounterResetPtr CounterReset;
	FUniformBufferPtr TriangleCullCommands;

	FGPUCommandSignaturePtr GPUCommandSignature;
	FGPUCommandBufferPtr GPUCommandBuffer;
};
typedef TI_INTRUSIVE_PTR(FGPUClusterCullCS) FGPUClusterCullCSPtr;
