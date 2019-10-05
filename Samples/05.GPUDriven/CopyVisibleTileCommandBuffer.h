/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FCopyCommandsParams)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, Info)
END_UNIFORM_BUFFER_STRUCT(FCopyCommandsParams)

class FCopyVisibleTileCommandBuffer : public FComputeTask
{
public:
	FCopyVisibleTileCommandBuffer();
	virtual ~FCopyVisibleTileCommandBuffer();

	void PrepareResources(FRHI * RHI);
	void UpdateComputeArguments(
		FRHI * RHI, 
		FScene * Scene, 
		FUniformBufferPtr TileVisibleInfo, 
		FUniformBufferPtr PrimitiveMetaInfo, 
		FGPUCommandBufferPtr CommandBuffer);
	virtual void Run(FRHI * RHI) override;
private:

private:
	FCopyCommandsParamsPtr CopyParams;

	FRenderResourceTablePtr ResourceTable;
};
typedef TI_INTRUSIVE_PTR(FCopyVisibleTileCommandBuffer) FCopyVisibleTileCommandBufferPtr;
