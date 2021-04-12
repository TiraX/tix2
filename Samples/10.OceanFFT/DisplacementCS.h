/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

BEGIN_UNIFORM_BUFFER_STRUCT(FDisplacementUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)		// x = Size
END_UNIFORM_BUFFER_STRUCT(FDisplacementUniform)


class FDisplacementCS : public FComputeTask
{
public:
	FDisplacementCS();
	virtual ~FDisplacementCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FTexturePtr InIFFT_X,
		FTexturePtr InIFFT_Y,
		FTexturePtr InIFFT_Z,
		FTexturePtr InJACOB_X,
		FTexturePtr InJACOB_Y,
		FTexturePtr InJACOB_Z
	);

	FTexturePtr GetDisplacementTexture()
	{
		return DisplacementTexture;
	}

private:
	enum
	{
		SRV_IFFT_X,
		SRV_IFFT_Y,
		SRV_IFFT_Z,
		SRV_JACOB_X,
		SRV_JACOB_Y,
		SRV_JACOB_Z,
		UAV_DISPLACEMENT_TEXTURE,

		PARAM_TOTAL_COUNT,
	};

private:
	FDisplacementUniformPtr InfoUniform;

	FRenderResourceTablePtr ResourceTable;

	FTexturePtr IFFTTextures[3];
	FTexturePtr JacobTextures[3];
	FTexturePtr DisplacementTexture;
};
typedef TI_INTRUSIVE_PTR(FDisplacementCS) FDisplacementCSPtr;
