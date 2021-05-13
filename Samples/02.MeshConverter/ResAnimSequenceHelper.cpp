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
		ChunkHeader.ID = TIRES_ID_CHUNK_ANIMS;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_ANIM;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderAnimSequence Define;
			Define.NumFrames = TotalFrames;
			Define.NumTracks = TotalTracks;
			HeaderStream.Put(&Define, sizeof(THeaderAnimSequence));

			int32 Offset = 0;
			for (int32 t = 0; t < TotalTracks; ++t)
			{
				const FTrackInfo& Track = Tracks[t];
				THeaderTrack HeaderTrack;
				HeaderTrack.RefBoneIndex = Track.RefBoneIndex;
				HeaderTrack.KeyDataOffset = Offset;
				HeaderTrack.NumPosKeys = (int32)(Track.PosKeys.size() / 3);
				HeaderTrack.NumRotKeys = (int32)(Track.RotKeys.size() / 4);
				HeaderTrack.NumScaleKeys = (int32)(Track.ScaleKeys.size() / 3);
				Offset += HeaderTrack.NumPosKeys * 3 + HeaderTrack.NumRotKeys * 4 + HeaderTrack.NumScaleKeys * 3;

				HeaderStream.Put(&HeaderTrack, sizeof(THeaderTrack));
				if (!Track.PosKeys.empty())
					DataStream.Put(Track.PosKeys.data(), (uint32)(Track.PosKeys.size() * sizeof(float)));
				if (!Track.RotKeys.empty())
					DataStream.Put(Track.RotKeys.data(), (uint32)(Track.RotKeys.size() * sizeof(float)));
				if (!Track.ScaleKeys.empty())
					DataStream.Put(Track.ScaleKeys.data(), (uint32)(Track.ScaleKeys.size() * sizeof(float)));
			}
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
