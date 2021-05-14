/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TAnimSequence.h"

namespace tix
{
	TAnimSequence::TAnimSequence()
		: TResource(ERES_ANIM_SEQUENCE)
		, NumFrames(0)
		, SequenceLength(0.f)
		, RateScale(1.f)
	{
	}

	TAnimSequence::TAnimSequence(int32 InFrames, float InLength, float InRateScale, int32 InTracks)
		: TResource(ERES_ANIM_SEQUENCE)
		, NumFrames(InFrames)
		, SequenceLength(InLength)
		, RateScale(InRateScale)
	{
		Tracks.reserve(InTracks);
	}

	TAnimSequence::~TAnimSequence()
	{
	}

	void TAnimSequence::AddTrack(const TTrackInfo& InTrack)
	{
		Tracks.push_back(InTrack);
	}

	void TAnimSequence::SetFrameData(int32 InNumData, const float* InData)
	{
		FrameData.resize(InNumData);
		memcpy(FrameData.data(), InData, InNumData * sizeof(float));
	}
}