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
		FInstanceBufferPtr SceneInstanceData);
	virtual void Run(FRHI * RHI) override;

	FGPUCommandBufferPtr GetDispatchCommandBuffer()
	{
		return GPUCommandBuffer;
	}

	void SetMeshBuffer(FMeshBufferPtr InMB)
	{
		MeshBuffer = InMB;
	}
private:

private:
	FCullUniformPtr CullUniform;
	FRenderResourceTablePtr ResourceTable;

	FInstanceBufferPtr SceneInstanceData;

	FCounterResetPtr CounterReset;
	FUniformBufferPtr TriangleCullResults;
	FUniformBufferPtr DebugGroup;

	FGPUCommandSignaturePtr GPUCommandSignature;
	FGPUCommandBufferPtr GPUCommandBuffer;

	// Hack meshbuffers
	FMeshBufferPtr MeshBuffer;

};
typedef TI_INTRUSIVE_PTR(FGPUTriangleCullCS) FGPUTriangleCullCSPtr;
