/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FHiZDownSampleInfo)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FUInt4, RTInfo)	// xy = Dest RT Size, z = Source RT MipLevel
END_UNIFORM_BUFFER_STRUCT(FHiZDownSampleInfo)
class FHiZDownSampleCS : public FComputeTask
{
public:
	static const uint32 HiZLevels = 8;

	FHiZDownSampleCS();
	virtual ~FHiZDownSampleCS();

	void PrepareResources(FRHI * RHI, const vector2di& HiZSize, FTexturePtr DepthTexture);
	void UpdateComputeArguments(FRHI * RHI, uint32 InLevel);
	virtual void Run(FRHI * RHI) override;

private:

private:
	// Target RT Size
	FHiZDownSampleInfoPtr InfoUniforms[HiZLevels];
	
	// Input Depth texture mip and Ouput depth texture mip
	FRenderResourceTablePtr ResourceTable[HiZLevels];

	uint32 ActiveLevel;
};
typedef TI_INTRUSIVE_PTR(FHiZDownSampleCS) FHiZDownSampleCSPtr;
