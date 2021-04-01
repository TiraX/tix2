/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class TMACGrid
{
public:
	TMACGrid();
	~TMACGrid();

	void InitSize(const vector3di& InSize, float InSeperation);

	const vector3di& GetSize() const
	{
		return Size;
	}

	float GetSeperation() const
	{
		return Seperation;
	}

protected:
	vector3di Size;
	float Seperation;

	TVector<float> U, V, W;
};