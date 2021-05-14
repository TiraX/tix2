/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResAnimSequenceHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	TResAnimSequenceHelper::TResAnimSequenceHelper()
		: TotalFrames(0)
		, TotalTracks(0)
	{
	}

	TResAnimSequenceHelper::~TResAnimSequenceHelper()
	{
	}

	void TResAnimSequenceHelper::LoadAnimSequence(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResAnimSequenceHelper Helper;
		// Bones
		Helper.TotalFrames = Doc["total_frames"].GetInt();
		Helper.SequenceLength = Doc["sequence_length"].GetFloat();
		TI_ASSERT(Helper.SequenceLength > 0.f);
		Helper.RateScale = Doc["rate_scale"].GetFloat();
		TI_ASSERT(Helper.RateScale > 0.f);
		Helper.TotalTracks = Doc["total_tracks"].GetInt();
		Helper.RefSkeleton = Doc["ref_skeleton"].GetString();

		TJSONNode JTracks = Doc["tracks"];
		TI_ASSERT(JTracks.IsArray() && JTracks.Size() == Helper.TotalTracks);

		Helper.Tracks.resize(Helper.TotalTracks);
		for (int32 t = 0; t < Helper.TotalTracks; t++)
		{
			TJSONNode JTrack = JTracks[t];
			FTrackInfo& Info = Helper.Tracks[t];
			Info.RefBoneIndex = JTrack["ref_bone_index"].GetInt();

			ConvertJArrayToArray(JTrack["pos_keys"], Info.PosKeys);
			ConvertJArrayToArray(JTrack["rot_keys"], Info.RotKeys);
			ConvertJArrayToArray(JTrack["scale_keys"], Info.ScaleKeys);

			TI_ASSERT(Info.PosKeys.size() == 0 || Info.PosKeys.size() == 3 || Info.PosKeys.size() == Helper.TotalFrames * 3);
			TI_ASSERT(Info.RotKeys.size() == 0 || Info.RotKeys.size() == 4 || Info.RotKeys.size() == Helper.TotalFrames * 4);
			TI_ASSERT(Info.ScaleKeys.size() == 0 || Info.ScaleKeys.size() == 3 || Info.ScaleKeys.size() == Helper.TotalFrames * 3);
		}
		Helper.OutputAnimSequence(OutStream, OutStrings);
	}
	
	void TResAnimSequenceHelper::OutputAnimSequence(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_ANIMATION;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_ANIM;
		ChunkHeader.ElementCount = 1;

		THeaderAnimSequence Define;
		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			Define.NumFrames = TotalFrames;
			Define.Length = SequenceLength;
			Define.RateScale = RateScale;
			Define.NumTracks = TotalTracks;
			Define.NumData = 0;
			// Do not put into header stream now, since we want to upadte NumData
			//HeaderStream.Put(&Define, sizeof(THeaderAnimSequence));

			int32 Offset = 0;
			for (int32 t = 0; t < TotalTracks; ++t)
			{
				const FTrackInfo& Track = Tracks[t];
				TTrackInfo TrackInfo;
				TrackInfo.RefBoneIndex = Track.RefBoneIndex;
				TrackInfo.KeyDataOffset = Offset;
				TrackInfo.NumPosKeys = (int32)(Track.PosKeys.size() / 3);
				TrackInfo.NumRotKeys = (int32)(Track.RotKeys.size() / 4);
				TrackInfo.NumScaleKeys = (int32)(Track.ScaleKeys.size() / 3);
				Offset += TrackInfo.NumPosKeys * 3 + TrackInfo.NumRotKeys * 4 + TrackInfo.NumScaleKeys * 3;
				HeaderStream.Put(&TrackInfo, sizeof(TTrackInfo));

				if (!Track.PosKeys.empty())
					DataStream.Put(Track.PosKeys.data(), (uint32)(Track.PosKeys.size() * sizeof(float)));
				if (!Track.RotKeys.empty())
					DataStream.Put(Track.RotKeys.data(), (uint32)(Track.RotKeys.size() * sizeof(float)));
				if (!Track.ScaleKeys.empty())
					DataStream.Put(Track.ScaleKeys.data(), (uint32)(Track.ScaleKeys.size() * sizeof(float)));
			}
			Define.NumData = Offset;
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(&Define, sizeof(THeaderAnimSequence));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		FillZero4(OutStream);
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
