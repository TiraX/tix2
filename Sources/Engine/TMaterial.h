/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TMaterialRTInfo
	{
		int32 NumRT;
		E_PIXEL_FORMAT ColorRT[ERTC_COUNT];
		E_PIXEL_FORMAT DepthRT;

		TMaterialRTInfo()
			: NumRT(0)
		{
			ColorRT[ERTC_COLOR0] = EPF_UNKNOWN;
			ColorRT[ERTC_COLOR1] = EPF_UNKNOWN;
			ColorRT[ERTC_COLOR2] = EPF_UNKNOWN;
			ColorRT[ERTC_COLOR3] = EPF_UNKNOWN;

			DepthRT = EPF_UNKNOWN;
		}
	};

	class TMaterial : public TResource
	{
	public:
		TMaterial();
		virtual ~TMaterial();

		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

		TPipelinePtr Pipeline;
		
		enum E_BLEND_MODE
		{
			MATERIAL_BLEND_OPAQUE,
			MATERIAL_BLEND_TRANSLUCENT,
			MATERIAL_BLEND_MASK,
			MATERIAL_BLEND_ADDITIVE,
		};
		void SetShaderName(E_SHADER_STAGE Stage, const TString& Name);
		void SetShaderCode(E_SHADER_STAGE Stage, const uint8* CodeBuffer, int32 Length);
		void SetBlendMode(E_BLEND_MODE InBlendMode);
		void SetShaderVsFormat(uint32 InVsFormat);
		void EnableDepthWrite(bool bEnable);
		void EnableDepthTest(bool bEnable);
		void EnableTwoSides(bool bEnable);

		// temp method
		TI_API void SetRTColorBufferCount(int32 Count);
		TI_API void SetRTColor(E_PIXEL_FORMAT Format, E_RT_COLOR_BUFFER ColorBuffer);
		TI_API void SetRTDepth(E_PIXEL_FORMAT Format);

	private:
		TString ShaderNames[ESS_COUNT];
		E_BLEND_MODE BlendMode;
		uint32 VsFormat;
		bool bDepthWrite;
		bool bDepthTest;
		bool bTwoSides;

		TMaterialRTInfo RTInfo;

		TStream ShaderCodes[ESS_COUNT];

		friend class TPipeline;
		friend class TResFile;
	};
}
