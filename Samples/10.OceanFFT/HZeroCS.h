/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

BEGIN_UNIFORM_BUFFER_STRUCT(FHZeroUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)		//x = Size; y = L; z = windSpeed ^ 2 / g; w = A;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info1)	// xy = normalized_wind_direction; z = l(spress value); w = damp;
END_UNIFORM_BUFFER_STRUCT(FHZeroUniform)


class FHZeroCS : public FComputeTask
{
public:
	FHZeroCS();
	virtual ~FHZeroCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FTexturePtr InGaussTexture,
		float InWaveHeight,
		float InWindSpeed,
		const vector2df& InWindDir,
		float InSpressWaveLength,
		float InDamp
	);

private:
	enum
	{
		SRV_GAUSS_RANDOM,
		UAV_H0_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:
	FHZeroUniformPtr InfoUniform;

	FRenderResourceTablePtr ResourceTable;

	FTexturePtr GaussTexture;
	FTexturePtr H0Texture;
};
typedef TI_INTRUSIVE_PTR(FHZeroCS) FHZeroCSPtr;
