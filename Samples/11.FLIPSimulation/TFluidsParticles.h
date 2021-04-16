/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TFluidsParticles
{
public:
	TFluidsParticles();
	~TFluidsParticles();

	void InitWithShapeSphere(const vectype& InCenter, float InRadius, float InSeperation);
	void ExportToJson(const TString& Filename);

	struct TParticle
	{
		vectype Position;
		vectype Velocity;
	};
protected:
	TVector<TParticle> Particles;

	friend class TFlipSolver;
};