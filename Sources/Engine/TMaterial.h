/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_BLEND_MODE
	{
		BLEND_MODE_OPAQUE,
		BLEND_MODE_TRANSLUCENT,
		BLEND_MODE_MASK,
		BLEND_MODE_ADDITIVE,

		BLEND_MODE_COUNT,
	};

	//struct TMaterialRTInfo
	//{
	//	int32 NumRT;
	//	E_PIXEL_FORMAT ColorRT[ERTC_COUNT];
	//	E_PIXEL_FORMAT DepthRT;

	//	TMaterialRTInfo()
	//		: NumRT(0)
	//	{
	//		ColorRT[ERTC_COLOR0] = EPF_UNKNOWN;
	//		ColorRT[ERTC_COLOR1] = EPF_UNKNOWN;
	//		ColorRT[ERTC_COLOR2] = EPF_UNKNOWN;
	//		ColorRT[ERTC_COLOR3] = EPF_UNKNOWN;

	//		DepthRT = EPF_UNKNOWN;
	//	}
	//};

	class TMaterial : public TPipeline
	{
	public:
		TMaterial();
		virtual ~TMaterial();

		void SetShaderName(E_SHADER_STAGE Stage, const TString& Name);
		void SetShaderCode(E_SHADER_STAGE Stage, const uint8* CodeBuffer, int32 Length);
		void SetShaderCode(E_SHADER_STAGE Stage, TFile& File);
		void SetBlendMode(E_BLEND_MODE InBlendMode);
		void SetShaderVsFormat(uint32 InVsFormat);
		void EnableDepthWrite(bool bEnable);
		void EnableDepthTest(bool bEnable);
		void EnableTwoSides(bool bEnable);

		void EnableState(E_PIPELINE_STATES_OPTION InState, bool bEnable);
		void SetBlendState(const TBlendState& InBlendState);
		void SetRasterizerState(const TRasterizerDesc& InRasterizerState);
		void SetDepthStencilState(const TDepthStencilDesc& InDepthStencilState);

		void SetRTColorBufferCount(int32 Count);
		void SetRTColor(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBuffer);
		void SetRTDepth(E_PIXEL_FORMAT Format);

	private:
		TString ShaderNames[ESS_COUNT];

		//TMaterialRTInfo RTInfo;

		friend class TPipeline;
		friend class TResFile;
	};
}
