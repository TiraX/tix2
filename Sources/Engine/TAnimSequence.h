/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	struct TTrackInfo
	{
		int32 RefBoneIndex;
		int32 KeyDataOffset;	// Count by float (not byte)
		int32 NumPosKeys;
		int32 NumRotKeys;
		int32 NumScaleKeys;
	};

	// TAnimSequence, hold animation key frames data
	class TI_API TAnimSequence : public TResource
	{
	public:
		TAnimSequence();
		TAnimSequence(int32 InFrames, float InLength, float InRateScale, int32 InTracks);
		~TAnimSequence();

		void AddTrack(const TTrackInfo& InTrack);
		void SetFrameData(int32 InNumData, const float* InData);

		virtual void InitRenderThreadResource() override {};
		virtual void DestroyRenderThreadResource() override {};

	protected:

	protected:
		int32 NumFrames;
		float SequenceLength;
		float RateScale;

		TVector<TTrackInfo> Tracks;
		TVector<float> FrameData;
	};
}
