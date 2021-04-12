/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

BEGIN_UNIFORM_BUFFER_STRUCT(FGenerateNormalUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)		// // xy = Size
END_UNIFORM_BUFFER_STRUCT(FGenerateNormalUniform)


class FGenerateNormalCS : public FComputeTask
{
public:
	FGenerateNormalCS();
	virtual ~FGenerateNormalCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FTexturePtr InDisplacement
	);

	FTexturePtr GetNormalTexture()
	{
		return NormalTexture;
	}

private:
	enum
	{
		SRV_DISPLACEMENT,
		UAV_NORMAL_TEXTURE,

		PARAM_TOTAL_COUNT,
	};

private:
	FGenerateNormalUniformPtr InfoUniform;

	FRenderResourceTablePtr ResourceTable;
;
	FTexturePtr DisplacementTexture;
	FTexturePtr NormalTexture;
};
typedef TI_INTRUSIVE_PTR(FGenerateNormalCS) FGenerateNormalCSPtr;
