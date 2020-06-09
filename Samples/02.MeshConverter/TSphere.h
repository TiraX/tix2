/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

struct TSphere
{
	vector3df Center;
	float Radius;

	TSphere()
		: Radius(0.f)
	{}

	TSphere(const vector3df& InCenter, float InRadius)
		: Center(InCenter)
		, Radius(InRadius)
	{}

	bool IsPointInsideSphere(const vector3df& P) const
	{
		return (Center - P).getLength() <= Radius;
	}

	bool operator == (const TSphere& Other) const
	{
		return Center == Other.Center && Radius == Other.Radius;
	}
};