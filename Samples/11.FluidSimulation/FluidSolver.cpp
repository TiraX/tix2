/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolver.h"
#include "FluidSimRenderer.h"

FFluidSolver::FFluidSolver()
	: Flag(0)
	, TotalParticles(0)
	, ParticleMass(1.f)
	, ParticleSeperation(0.1f)
	, RestDenstiy(1000.f)
	, TimeStep(1.f / 60.f / 2.f)
	, Epsilon(600.f)
	, Iterations(3)
	, TotalCells(0)
	, CellSize(1.f)
{
	UB_PbfParams = ti_new FPBFParams;
	UB_Boundary = ti_new FBoundaryInfo;

	// Compute tasks
	FRHI* RHI = FRHI::Get();
	CellInitCS = ti_new FCellInitCS;
	CellInitCS->Finalize();
	CellInitCS->PrepareResources(RHI);
	P2CellCS = ti_new FP2CellCS;
	P2CellCS->Finalize();
	P2CellCS->PrepareResources(RHI);
	CalcOffsetsCS = ti_new FCalcOffsetsCS;
	CalcOffsetsCS->Finalize();
	CalcOffsetsCS->PrepareResources(RHI);
	SortCS = ti_new FSortCS;
	SortCS->Finalize();
	SortCS->PrepareResources(RHI);
	ApplyGravityCS = ti_new FApplyGravityCS;
	ApplyGravityCS->Finalize();
	ApplyGravityCS->PrepareResources(RHI);
	NeighborSearchCS = ti_new FNeighborSearchCS;
	NeighborSearchCS->Finalize();
	NeighborSearchCS->PrepareResources(RHI);
	LambdaCS = ti_new FLambdaCS;
	LambdaCS->Finalize();
	LambdaCS->PrepareResources(RHI);
	DeltaPosCS = ti_new FDeltaPosCS;
	DeltaPosCS->Finalize();
	DeltaPosCS->PrepareResources(RHI);
	ApplyDeltaPosCS = ti_new FApplyDeltaPosCS;
	ApplyDeltaPosCS->Finalize();
	ApplyDeltaPosCS->PrepareResources(RHI);
	UpdateVelocityCS = ti_new FUpdateVelocityCS;
	UpdateVelocityCS->Finalize();
	UpdateVelocityCS->PrepareResources(RHI);
}

FFluidSolver::~FFluidSolver()
{
}

void FFluidSolver::CreateParticlesInBox(
	const aabbox3df& InParticleBox,
	float InParticleSeperation,
	float InParticleMass,
	float InRestDenstiy)
{
	// Calc Total Particles
	const float dis = ParticleSeperation * 0.9f;
	vector3df Ext = InParticleBox.getExtent();
	vector3di ParticleDim;
	ParticleDim.X = (int32)(Ext.X / dis);
	ParticleDim.Y = (int32)(Ext.Y / dis);
	ParticleDim.Z = (int32)(Ext.Z / dis);
	TotalParticles = ParticleDim.X * ParticleDim.Y * ParticleDim.Z;

	// Update params
	ParticleMass = InParticleMass;
	ParticleSeperation = InParticleSeperation;
	RestDenstiy = InRestDenstiy;
	Flag |= DirtyParams;


	// Create particles and resources
	ParticlePositions.reserve(TotalParticles);
	const float jitter = 0.f;// ParticleSeperation * 0.5f;
	TMath::RandSeed(12306);
	for (float z = InParticleBox.MinEdge.Z; z < InParticleBox.MaxEdge.Z; z += dis)
	{
		for (float y = InParticleBox.MinEdge.Y; y < InParticleBox.MaxEdge.Y; y += dis)
		{
			for (float x = InParticleBox.MinEdge.X; x < InParticleBox.MaxEdge.X; x += dis)
			{
				vector3df Pos;
				Pos.X = x + TMath::RandomUnit() * jitter;
				Pos.Y = y + TMath::RandomUnit() * jitter;
				Pos.Z = z + TMath::RandomUnit() * jitter;
				ParticlePositions.push_back(Pos);
			}
		}
	}
	TotalParticles = (int32)ParticlePositions.size();
	Flag |= DirtyParticles;
}

void FFluidSolver::SetBoundaryBox(const aabbox3df& InBoundaryBox)
{
	BoundaryBox = InBoundaryBox;
	CellSize = ParticleSeperation;
	const float CellSizeInv = 1.f / CellSize;
	
	vector3df BoundExt = BoundaryBox.getExtent();
	Dim.X = (int32)(BoundExt.X * CellSizeInv + 1);
	Dim.Y = (int32)(BoundExt.Y * CellSizeInv + 1);
	Dim.Z = (int32)(BoundExt.Z * CellSizeInv + 1);
	
	TotalCells = Dim.X * Dim.Y * Dim.Z;

	Flag |= DirtyBoundary;
}

void FFluidSolver::UpdateParamBuffers(FRHI* RHI)
{
	if ((Flag & DirtyParams) != 0)
	{
		// P0: x = mass; y = epsilon; z = m/rho; w = dt
		// P1: x = h; y = h^2; z = 1.f/(h^3) w = inv_cell_size;
		// Dim: xyz = Dim, w = TotalParticles
		UB_PbfParams->UniformBufferData[0].P0 = FFloat4(ParticleMass, Epsilon, ParticleMass / RestDenstiy, TimeStep);
		const float h = ParticleSeperation;
		// Calc cell size
		UB_PbfParams->UniformBufferData[0].P1 = FFloat4(h, h * h, 1.f / (h * h * h), 1.f / CellSize);
		UB_PbfParams->UniformBufferData[0].Dim = FInt4(Dim.X, Dim.Y, Dim.Z, TotalParticles);
		UB_PbfParams->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
	}

	if ((Flag & DirtyBoundary) != 0)
	{
		// Boundary Uniforms
		UB_Boundary->UniformBufferData[0].Min = FFloat4(BoundaryBox.MinEdge.X, BoundaryBox.MinEdge.Y, BoundaryBox.MinEdge.Z, 1.f);
		UB_Boundary->UniformBufferData[0].Max = FFloat4(BoundaryBox.MaxEdge.X, BoundaryBox.MaxEdge.Y, BoundaryBox.MaxEdge.Z, 1.f);
		UB_Boundary->InitUniformBuffer(UB_FLAG_INTERMEDIATE);
	}
}

void FFluidSolver::UpdateResourceBuffers(FRHI * RHI)
{
	/** Particles with Position and Velocity */
	if ((Flag & DirtyParticles) != 0)
	{
		TI_ASSERT(ParticlePositions.size() == TotalParticles);
		UB_ParticlePositions = RHI->CreateUniformBuffer(sizeof(vector3df), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_ParticlePositions->SetResourceName("ParticlePositions");
		RHI->UpdateHardwareResourceUB(UB_ParticlePositions, ParticlePositions.data());
	}
	if (UB_ParticleVelocities == nullptr || UB_ParticleVelocities->GetElements() != TotalParticles)
	{
		UB_ParticleVelocities = RHI->CreateUniformBuffer(sizeof(vector3df), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_ParticleVelocities->SetResourceName("ParticleVelocities");
		RHI->UpdateHardwareResourceUB(UB_ParticleVelocities, nullptr);
		UB_SortedPositions = RHI->CreateUniformBuffer(sizeof(vector3df), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_SortedPositions->SetResourceName("SortedPositions");
		RHI->UpdateHardwareResourceUB(UB_SortedPositions, nullptr);
		UB_SortedVelocities = RHI->CreateUniformBuffer(sizeof(vector3df), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_SortedVelocities->SetResourceName("SortedVelocities");
		RHI->UpdateHardwareResourceUB(UB_SortedVelocities, nullptr);
	}

	// Cells
	if (UB_NumInCell == nullptr || UB_NumInCell->GetElements() != TotalCells)
	{
		UB_NumInCell = RHI->CreateUniformBuffer(sizeof(uint32), TotalCells, UB_FLAG_COMPUTE_WRITABLE);
		UB_NumInCell->SetResourceName("NumInCell");
		RHI->UpdateHardwareResourceUB(UB_NumInCell, nullptr);

		UB_CellParticles = RHI->CreateUniformBuffer(sizeof(uint32), TotalCells * MaxParticleInCell, UB_FLAG_COMPUTE_WRITABLE);
		UB_CellParticles->SetResourceName("CellParticles");
		RHI->UpdateHardwareResourceUB(UB_CellParticles, nullptr);

		UB_CellParticleOffsets = RHI->CreateUniformBuffer(sizeof(uint32), TotalCells, UB_FLAG_COMPUTE_WRITABLE);
		UB_CellParticleOffsets->SetResourceName("CellParticleOffsets");
		RHI->UpdateHardwareResourceUB(UB_CellParticleOffsets, nullptr);
	}

	// Particles
	if (UB_PositionOld == nullptr || UB_PositionOld->GetElements() != TotalParticles)
	{
		UB_PositionOld = RHI->CreateUniformBuffer(sizeof(vector3df), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_PositionOld->SetResourceName("PositionOld");
		RHI->UpdateHardwareResourceUB(UB_PositionOld, nullptr);

		UB_NeighborNum = RHI->CreateUniformBuffer(sizeof(uint32), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_NeighborNum->SetResourceName("NeighborNum");
		RHI->UpdateHardwareResourceUB(UB_NeighborNum, nullptr);

		UB_NeighborParticles = RHI->CreateUniformBuffer(sizeof(uint32), TotalParticles * MaxNeighbors, UB_FLAG_COMPUTE_WRITABLE);
		UB_NeighborParticles->SetResourceName("NeighborParticles");
		RHI->UpdateHardwareResourceUB(UB_NeighborParticles, nullptr);

		UB_Lambdas = RHI->CreateUniformBuffer(sizeof(float), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_Lambdas->SetResourceName("Lambdas");
		RHI->UpdateHardwareResourceUB(UB_Lambdas, nullptr);

		UB_DeltaPositions = RHI->CreateUniformBuffer(sizeof(vector3df), TotalParticles, UB_FLAG_COMPUTE_WRITABLE);
		UB_DeltaPositions->SetResourceName("DeltaPositions");
		RHI->UpdateHardwareResourceUB(UB_DeltaPositions, nullptr);
	}
}

void FFluidSolver::UpdateComputeParams(FRHI* RHI)
{
	CellInitCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_NumInCell,
		UB_CellParticleOffsets); 
	P2CellCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_ParticlePositions,
		UB_NumInCell,
		UB_CellParticles);
	CalcOffsetsCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_NumInCell,
		UB_CellParticleOffsets);

	SortCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_ParticlePositions,
		UB_ParticleVelocities,
		UB_NumInCell,
		UB_CellParticleOffsets,
		UB_CellParticles,
		UB_SortedPositions,
		UB_SortedVelocities);
	ApplyGravityCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_SortedPositions,
		UB_SortedVelocities,
		UB_PositionOld);
	NeighborSearchCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_NumInCell,
		UB_CellParticleOffsets,
		UB_SortedPositions,
		UB_NeighborNum,
		UB_NeighborParticles);
	LambdaCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_SortedPositions,
		UB_NeighborNum,
		UB_NeighborParticles,
		UB_Lambdas);
	DeltaPosCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_NeighborNum,
		UB_NeighborParticles,
		UB_Lambdas,
		UB_SortedPositions,
		UB_DeltaPositions);
	ApplyDeltaPosCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_DeltaPositions,
		UB_SortedPositions);
	UpdateVelocityCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_PositionOld,
		UB_SortedPositions,
		UB_SortedVelocities);
}

void FFluidSolver::Update(FRHI * RHI, float Dt)
{
	// Update uniform buffers
	if (Flag != 0)
	{
		UpdateParamBuffers(RHI);
		UpdateResourceBuffers(RHI);

		UpdateComputeParams(RHI);
		Flag = 0;
	}

	RHI->BeginComputeTask();
	{
		RHI->BeginEvent("CellInit");
		RHI->SetResourceStateUB(UB_NumInCell, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->SetResourceStateUB(UB_CellParticleOffsets, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		CellInitCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("P2Cell");
		RHI->SetResourceStateUB(UB_ParticlePositions, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_NumInCell, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->SetResourceStateUB(UB_CellParticles, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		P2CellCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("CalcOffsets");
		RHI->SetResourceStateUB(UB_NumInCell, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->SetResourceStateUB(UB_CellParticleOffsets, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		CalcOffsetsCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("Sort");
		RHI->SetResourceStateUB(UB_ParticlePositions, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_ParticleVelocities, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_NumInCell, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_CellParticleOffsets, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_CellParticles, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->SetResourceStateUB(UB_SortedVelocities, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		SortCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("ApplyGravity");
		RHI->SetResourceStateUB(UB_SortedVelocities, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->SetResourceStateUB(UB_PositionOld, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		ApplyGravityCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("NeighborSearch");
		RHI->SetResourceStateUB(UB_NumInCell, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_CellParticleOffsets, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_NeighborNum, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->SetResourceStateUB(UB_NeighborParticles, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		NeighborSearchCS->Run(RHI);
		RHI->EndEvent();

		for (int32 iter = 0; iter < Iterations; ++iter)
		{
			RHI->BeginEvent("Lambda", iter);
			RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_NeighborNum, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_NeighborParticles, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_Lambdas, RESOURCE_STATE_UNORDERED_ACCESS, false);
			RHI->FlushResourceStateChange();
			LambdaCS->Run(RHI);
			RHI->EndEvent();

			RHI->BeginEvent("DeltaPos", iter);
			RHI->SetResourceStateUB(UB_NeighborNum, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_NeighborParticles, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_Lambdas, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_DeltaPositions, RESOURCE_STATE_UNORDERED_ACCESS, false);
			RHI->FlushResourceStateChange();
			DeltaPosCS->Run(RHI);
			RHI->EndEvent();

			RHI->BeginEvent("ApplyDeltaPos", iter);
			RHI->SetResourceStateUB(UB_DeltaPositions, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
			RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_UNORDERED_ACCESS, false);
			RHI->FlushResourceStateChange();
			ApplyDeltaPosCS->Run(RHI);
			RHI->EndEvent();
		}

		RHI->BeginEvent("UpdateVelocity");
		RHI->SetResourceStateUB(UB_PositionOld, RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, false);
		RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->SetResourceStateUB(UB_SortedVelocities, RESOURCE_STATE_UNORDERED_ACCESS, false);
		RHI->FlushResourceStateChange();
		UpdateVelocityCS->Run(RHI);
		RHI->EndEvent();

		// Copy sorted positions, velocities to particle positions and velocities
		if (!FFluidSimRenderer::PauseUpdate)
		{
			RHI->SetResourceStateUB(UB_ParticlePositions, RESOURCE_STATE_COPY_DEST, false);
			RHI->SetResourceStateUB(UB_ParticleVelocities, RESOURCE_STATE_COPY_DEST, false);
			RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_COPY_SOURCE, false);
			RHI->SetResourceStateUB(UB_SortedVelocities, RESOURCE_STATE_COPY_SOURCE, false);
			RHI->FlushResourceStateChange();
			RHI->CopyBufferRegion(UB_ParticlePositions, 0, UB_SortedPositions, UB_ParticlePositions->GetElements() * sizeof(vector3df));
			RHI->CopyBufferRegion(UB_ParticleVelocities, 0, UB_SortedVelocities, UB_ParticleVelocities->GetElements() * sizeof(vector3df));
		}
		else
		{
			// Pause state change, always simulate the previous data
			if (FFluidSimRenderer::StepNext)
			{
				// Step next data
				RHI->SetResourceStateUB(UB_ParticlePositions, RESOURCE_STATE_COPY_DEST, false);
				RHI->SetResourceStateUB(UB_ParticleVelocities, RESOURCE_STATE_COPY_DEST, false);
				RHI->SetResourceStateUB(UB_SortedPositions, RESOURCE_STATE_COPY_SOURCE, false);
				RHI->SetResourceStateUB(UB_SortedVelocities, RESOURCE_STATE_COPY_SOURCE, false);
				RHI->FlushResourceStateChange();
				RHI->CopyBufferRegion(UB_ParticlePositions, 0, UB_SortedPositions, UB_ParticlePositions->GetElements() * sizeof(vector3df));
				RHI->CopyBufferRegion(UB_ParticleVelocities, 0, UB_SortedVelocities, UB_ParticleVelocities->GetElements() * sizeof(vector3df));

				FFluidSimRenderer::StepNext = false;
			}
		}
	}
	RHI->EndComputeTask();
}