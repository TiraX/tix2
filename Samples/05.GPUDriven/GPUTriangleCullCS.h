/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FGPUTriangleCullCS : public FComputeTask
{
public:
	FGPUTriangleCullCS();
	virtual ~FGPUTriangleCullCS();

	void PrepareResources(FRHI * RHI, const vector2di& RTSize, FTexturePtr HiZTexture);
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
typedef TI_INTRUSIVE_PTR(FGPUTriangleCullCS) FGPUTriangleCullCSPtr;
