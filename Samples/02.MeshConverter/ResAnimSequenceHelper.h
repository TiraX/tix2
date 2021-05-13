/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TResAnimSequenceHelper
	{
	public:
		TResAnimSequenceHelper();
		~TResAnimSequenceHelper();

		static void LoadAnimSequence(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings);

		void OutputAnimSequence(TStream& OutStream, TVector<TString>& OutStrings);

	private:

	private:
		int32 TotalFrames;
		int32 TotalTracks;
		TString RefSkeleton;

		struct FTrackInfo
		{
			int32 RefBoneIndex;
			TVector<float> PosKeys;
			TVector<float> RotKeys;
			TVector<float> ScaleKeys;
		};
		TVector<FTrackInfo> Tracks;
	};
}