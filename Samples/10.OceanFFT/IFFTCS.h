/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

BEGIN_UNIFORM_BUFFER_STRUCT(FIFFTUniform)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, Info)		// x = Dir(0=Hor;1=Ver); y = Stage;
END_UNIFORM_BUFFER_STRUCT(FIFFTUniform)


class FIFFTCS : public FComputeTask
{
public:
	FIFFTCS();
	virtual ~FIFFTCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FTexturePtr InHKtTexture,
		FTexturePtr InButterFlyTexture
	);

private:
	enum
	{
		SRV_SORUCE_TEXTURE,
		SRV_BUTTER_FLY,
		UAV_RESULT_TEXTURE,

		PARAM_TOTAL_COUNT,
	};

private:
	TVector<FIFFTUniformPtr> InfoUniformHorizontal;
	TVector<FIFFTUniformPtr> InfoUniformVertical;

	// Two resource tables for PING-PONG switch
	enum
	{
		PING,
		PONG,
	};
	FRenderResourceTablePtr ResourceTables[2];

	FTexturePtr HKtTexture;
	FTexturePtr ButterFlyTexture;
	FTexturePtr OutputTexture;
};
typedef TI_INTRUSIVE_PTR(FIFFTCS) FIFFTCSPtr;
