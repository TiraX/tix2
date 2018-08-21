/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
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
	private:
		TString ShaderNames[ESS_COUNT];
		E_BLEND_MODE BlendMode;
		uint32 VsFormat;
		bool bDepthWrite;
		bool bDepthTest;
		bool bTwoSides;

		TStream ShaderCodes[ESS_COUNT];

		friend class TPipeline;
		friend class TResFile;
	};
}
