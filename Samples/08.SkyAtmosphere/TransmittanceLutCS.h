/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once 

class FTransmittanceLutCS : public FComputeTask
{
public:
	static const int32 LUT_W = 256;
	static const int32 LUT_H = 64;

	FTransmittanceLutCS();
	virtual ~FTransmittanceLutCS();

	void PrepareResources(FRHI* RHI);
	virtual void Run(FRHI* RHI) override;

	void UpdataComputeParams(
		FRHI* RHI,
		FUniformBufferPtr InAtmosphereParam
	);

	FTexturePtr GetTransmittanceLutTexture()
	{
		return TransmittanceLut;
	}

private:
	enum
	{
		UAV_LUT_RESULT,

		PARAM_TOTAL_COUNT,
	};

private:
	FRenderResourceTablePtr ResourceTable;

	FUniformBufferPtr AtmosphereParam;
	FTexturePtr TransmittanceLut;

};
typedef TI_INTRUSIVE_PTR(FTransmittanceLutCS) FTransmittanceLutCSPtr;
