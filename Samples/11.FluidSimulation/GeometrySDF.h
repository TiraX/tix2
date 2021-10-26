/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

class FBoxSDF;
class FSphereSDF;

class FGeometrySDF
{
public:
	FGeometrySDF()
		: Inverted(false)
	{};
	virtual ~FGeometrySDF() {};

	virtual void InvertSDF()
	{
		Inverted = !Inverted;
	}
	virtual float SampleSDFByPosition(const vector3df& Position) = 0;

	static FBoxSDF* CreateBoxSDF(const vector3df& MinPoint, const vector3df& MaxPoint, bool Invert);
	static FSphereSDF* CreateSphereSDF(const vector3df& Center, float Radius, bool Invert);

	const aabbox3df& GetBBox() const
	{
		return BBox;
	}
protected:
	bool Inverted;
	aabbox3df BBox;
};

//////////////////////////////////////////////////

class FBoxSDF : public FGeometrySDF
{
public:
	FBoxSDF(const vector3df& InMinPoint, const vector3df& InMaxPoint);
	virtual ~FBoxSDF();

	virtual float SampleSDFByPosition(const vector3df& Position) override;

private:
	vector3df MinPoint;
	vector3df MaxPoint;
};