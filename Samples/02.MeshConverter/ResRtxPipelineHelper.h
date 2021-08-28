/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResRtxPipelineHelper
	{
	public:
		TResRtxPipelineHelper();
		~TResRtxPipelineHelper();

		static void LoadRtxPipeline(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);
		void OutputRtxPipeline(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		//E_BLEND_MODE BlendMode;
		TString ShaderLibName;
		TStream ShaderBlob;

		TRtxPipelineDesc RtxDesc;
	};
}