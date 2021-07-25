/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "SplatVelCS.h"
#include "FluidSolverGrid2d.h"

FSplatVelCS::FSplatVelCS()
	: FComputeTask("S_SplatVelCS")
{
	UB_SplatParam = ti_new FSplatParam;
}

FSplatVelCS::~FSplatVelCS()
{
}

void FSplatVelCS::PrepareResources(FRHI * RHI)
{
	ResourceTable = RHI->CreateRenderResourceTable(PARAM_TOTAL_COUNT, EHT_SHADER_RESOURCE);
}

void FSplatVelCS::UpdateComputeParams(
	FRHI * RHI,
	FUniformBufferPtr InParam,
	const vector2df& MousePointUV,
	const vector2df& MouseMoveDir,
	const float RadiusScale,
	FTexturePtr InVelocity
)
{
	UBRef_Fluid2dParam = InParam;

	UB_SplatParam->UniformBufferData[0].SplatInfo0 =
		FFloat4(MousePointUV.X, MousePointUV.Y, MouseMoveDir.X, MouseMoveDir.Y);	
	UB_SplatParam->UniformBufferData[0].SplatInfo1 =
		FFloat4(TMath::RandomUnit(), TMath::RandomUnit(), TMath::RandomUnit(), RadiusScale);
	UB_SplatParam->InitUniformBuffer(UB_FLAG_INTERMEDIATE);

	ResourceTable->PutRWTextureInTable(InVelocity, 0, UAV_VELOCITY);
}

void FSplatVelCS::Run(FRHI * RHI)
{
	const uint32 BlockSize = 16;
	const uint32 DispatchSize = FFluidSolverGrid2d::SimResolution / BlockSize;

	RHI->SetComputePipeline(ComputePipeline);
	RHI->SetComputeConstantBuffer(0, UBRef_Fluid2dParam);
	RHI->SetComputeConstantBuffer(1, UB_SplatParam->UniformBuffer);
	RHI->SetComputeResourceTable(2, ResourceTable);
	RHI->DispatchCompute(vector3di(BlockSize, BlockSize, 1), vector3di(DispatchSize, DispatchSize, 1));
}