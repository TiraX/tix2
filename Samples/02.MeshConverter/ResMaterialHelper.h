/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResMaterialHelper
	{
	public:
		TResMaterialHelper();
		~TResMaterialHelper();

		static void LoadMaterial(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		void SetShaderName(E_SHADER_STAGE Stage, const TString& Name);
		void SetBlendMode(E_BLEND_MODE InBlendMode);
		void SetShaderVsFormat(uint32 InVsFormat);
		void SetShaderInsFormat(uint32 InInsFormat);
		void EnableDepthWrite(bool bEnable);
		void EnableDepthTest(bool bEnable);
		void EnableTwoSides(bool bEnable);

		void OutputMaterial(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		//E_BLEND_MODE BlendMode;
		TString ShaderNames[ESS_COUNT];
		TStream ShaderCodes[ESS_COUNT];

		E_BLEND_MODE BlendMode;
		TPipelineDesc PipelineDesc;
	};
}