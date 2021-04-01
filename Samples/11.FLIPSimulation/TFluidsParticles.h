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

	void InitWithShapeSphere(const vector3df& InCenter, float InRadius, float InSeperation);

protected:
	TVector<vector3df> Positions;
	TVector<vector3df> Velocities;
};