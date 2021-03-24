/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

BEGIN_UNIFORM_BUFFER_STRUCT(FHKtUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)		// x = Size; y = L; z = Depth; w = Time;
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info1)	// x = ChopScale;
END_UNIFORM_BUFFER_STRUCT(FHKtUniform)


class FHKtCS : public FComputeTask
{
public:
	FHKtCS();
	virtual ~FHKtCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FTexturePtr InH0Texture,
		float InDepth,
		float InTime,
		float InChoppyScale
	);

	FTexturePtr GetHKtResult(int32 Index)
	{
		TI_ASSERT(Index >= 0 && Index < 3);
		return HKtResults[Index];
	}

private:
	enum
	{
		SRV_H0,
		UAV_HKT_X,
		UAV_HKT_Y,
		UAV_HKT_Z,

		PARAM_TOTAL_COUNT,
	};

private:
	FHKtUniformPtr InfoUniform;

	FRenderResourceTablePtr ResourceTable;

	FTexturePtr H0Texture;
	FTexturePtr HKtResults[3];
};
typedef TI_INTRUSIVE_PTR(FHKtCS) FHKtCSPtr;
