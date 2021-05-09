/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "FluidSolver.h"

FFluidSolver::FFluidSolver()
	: Flag(0)
	, TotalParticles(0)
	, ParticleMass(1.f)
	, ParticleSeperation(0.1f)
	, RestDenstiy(1000.f)
	, TimeStep(1.f / 60.f / 2.f)
	, Epsilon(600.f)
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
	vector3df Ext = InParticleBox.getExtent();
	vector3di Dim;
	Dim.X = (int32)(Ext.X / ParticleSeperation);
	Dim.Y = (int32)(Ext.Y / ParticleSeperation);
	Dim.Z = (int32)(Ext.Z / ParticleSeperation);
	TotalParticles = Dim.X * Dim.Y * Dim.Z;

	// Update params
	ParticleMass = InParticleMass;
	ParticleSeperation = InParticleSeperation;
	RestDenstiy = InRestDenstiy;
	Flag |= DirtyParams;


	// Create particles and resources
	ParticlePositions.reserve(TotalParticles);
	const float jitter = ParticleSeperation * 0.5f;
	TMath::RandSeed(12306);
	for (int32 z = 0; z < Dim.Z; z++)
	{
		for (int32 y = 0; y < Dim.Y; y++)
		{
			for (int32 x = 0; x < Dim.X; x++)
			{
				vector3df Pos;
				Pos.X = x * ParticleSeperation + InParticleBox.MinEdge.X + TMath::RandomUnit() * jitter;
				Pos.Y = y * ParticleSeperation + InParticleBox.MinEdge.Y + TMath::RandomUnit() * jitter;
				Pos.Z = z * ParticleSeperation + InParticleBox.MinEdge.Z + TMath::RandomUnit() * jitter;
				ParticlePositions.push_back(Pos);
			}
		}
	}
	Flag |= DirtyParticles;
}

void FFluidSolver::SetBoundaryBox(const aabbox3df& InBoundaryBox)
{
	BoundaryBox = InBoundaryBox;
	CellSize = ParticleSeperation * 0.5f;
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
		UB_PbfParams->UniformBufferData[0].Dim = FInt4(Dim.X, Dim.Y, Dim.Z, 1);
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
	}
}

void FFluidSolver::UpdateComputeParams(FRHI* RHI)
{
	CellInitCS->UpdateComputeParams(RHI,
		UB_PbfParams->UniformBuffer,
		UB_Boundary->UniformBuffer,
		UB_NumInCell,
		UB_CellParticleOffsets); 

	TI_ASSERT(0);
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
		CellInitCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("P2Cell");
		P2CellCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("CalcOffsets");
		CalcOffsetsCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("Sort");
		SortCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("ApplyGravity");
		ApplyGravityCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("NeighborSearch");
		NeighborSearchCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("Lambda");
		LambdaCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("DeltaPos");
		DeltaPosCS->Run(RHI);
		RHI->EndEvent();

		RHI->BeginEvent("UpdateVelocity");
		UpdateVelocityCS->Run(RHI);
		RHI->EndEvent();
	}
	RHI->EndComputeTask();
}