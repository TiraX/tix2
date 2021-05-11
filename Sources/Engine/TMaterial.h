/*
	TiX Engine v2.0 Copyright (C) 2018~2021
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

		void SetShaderVsFormat(uint32 InVsFormat);
		void SetShaderInsFormat(uint32 InInsFormat);
		void SetPrimitiveType(E_PRIMITIVE_TYPE InPrimitiveType);
		void EnableDepthWrite(bool bEnable);
		void EnableDepthTest(bool bEnable);
		void EnableTwoSides(bool bEnable);

		void EnableState(E_PIPELINE_STATES_OPTION InState, bool bEnable);
		void SetBlendState(E_BLEND_MODE InBlendMode, const TBlendState& InBlendState);
		void SetRasterizerState(const TRasterizerDesc& InRasterizerState);
		void SetDepthStencilState(const TDepthStencilDesc& InDepthStencilState);

		void SetRTColorBufferCount(int32 Count);
		void SetRTColor(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBuffer);
		void SetRTDepth(E_PIXEL_FORMAT Format);

		E_BLEND_MODE GetBlendMode() const
		{
			return BlendMode;
		}

	private:
		E_BLEND_MODE BlendMode;
		friend class TPipeline;
		friend class TAssetFile;
	};
}
