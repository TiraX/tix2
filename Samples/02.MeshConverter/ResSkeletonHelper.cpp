/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResSkeletonHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	TResSkeletonHelper::TResSkeletonHelper()
		: TotalBones(0)
	{
	}

	TResSkeletonHelper::~TResSkeletonHelper()
	{
	}

	void TResSkeletonHelper::LoadSkeleton(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResSkeletonHelper Helper;

		// Bones
		Helper.TotalBones = Doc["total_bones"].GetInt();

		TJSONNode JBones = Doc["bones"];
		TI_ASSERT(JBones.IsArray() && JBones.Size() == Helper.TotalBones);

		Helper.Bones.resize(Helper.TotalBones);
		for (int32 b = 0; b < Helper.TotalBones; b++)
		{
			TJSONNode JBone = JBones[b];
			TBoneInfo& Info = Helper.Bones[b];
			Info.ParentIndex = JBone["parent_index"].GetInt();
			ConvertJArrayToVec3(JBone["translation"], Info.Pos);
			ConvertJArrayToQuat(JBone["rotation"], Info.Rot);
			ConvertJArrayToVec3(JBone["scale"], Info.Scale);
		}

		Helper.OutputSkeleton(OutStream, OutStrings);
	}
	
	void TResSkeletonHelper::OutputSkeleton(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_SKELETON;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_SKELETON;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderSkeleton Define;
			Define.NumBones = TotalBones;

			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderSkeleton));

			// Write bones
			DataStream.Put(Bones.data(), (uint32)(Bones.size() * sizeof(TBoneInfo)));
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
