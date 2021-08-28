/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResRtxPipelineHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	TResRtxPipelineHelper::TResRtxPipelineHelper()
	{
	}

	TResRtxPipelineHelper::~TResRtxPipelineHelper()
	{
	}

	void TResRtxPipelineHelper::LoadRtxPipeline(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResRtxPipelineHelper Helper;

		// shader lib
		TJSONNode JShaderLib = Doc["shader_lib"];
		Helper.ShaderLibName = JShaderLib.GetString();

		// export names
		TJSONNode JExportNames = Doc["export_names"];
		TI_ASSERT(JExportNames.IsArray());
		Helper.RtxDesc.ExportNames.resize(JExportNames.Size());
		for (int32 i = 0; i < JExportNames.Size(); ++i)
		{
			Helper.RtxDesc.ExportNames[i] = JExportNames[i].GetString();
		}

		// configs
		TJSONNode JMaxAttributeSizeInBytes = Doc["max_attribute_size_in_bytes"];
		TJSONNode JMaxPayloadSizeInBytes = Doc["max_payload_size_in_bytes"];
		TJSONNode JMaxTraceRecursionDepth = Doc["max_trace_recursion_depth"];
		Helper.RtxDesc.MaxAttributeSizeInBytes = JMaxAttributeSizeInBytes.GetInt();
		Helper.RtxDesc.MaxPayloadSizeInBytes = JMaxPayloadSizeInBytes.GetInt();
		Helper.RtxDesc.MaxTraceRecursionDepth = JMaxTraceRecursionDepth.GetInt();

		// hit group
		TJSONNode JHitGroupName = Doc["hit_group_name"];
		Helper.RtxDesc.HitGroupName = JHitGroupName.GetString();
		TJSONNode JHitGroup = Doc["hit_group"];
		TI_ASSERT(JHitGroup.IsArray() && JHitGroup.Size() == HITGROUP_NUM);
		Helper.RtxDesc.HitGroup[HITGROUP_ANY_HIT] = JHitGroup[HITGROUP_ANY_HIT].GetString();
		Helper.RtxDesc.HitGroup[HITGROUP_CLOSEST_HIT] = JHitGroup[HITGROUP_CLOSEST_HIT].GetString();
		Helper.RtxDesc.HitGroup[HITGROUP_INTERSECTION] = JHitGroup[HITGROUP_INTERSECTION].GetString();

		Helper.OutputRtxPipeline(OutStream, OutStrings);
	}
	
	void TResRtxPipelineHelper::OutputRtxPipeline(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_RTX_PIPELINE;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_RTX_PIPELINE;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderRtxPipeline Define;
			Define.ShaderLibName = AddStringToList(OutStrings, ShaderLibName);
			Define.NumExportNames = (int32)RtxDesc.ExportNames.size();
			Define.HitGroupName = AddStringToList(OutStrings, RtxDesc.HitGroupName);
			Define.HitGroupAnyHit = 
				RtxDesc.HitGroup[HITGROUP_ANY_HIT] == "" ? -1 : AddStringToList(OutStrings, RtxDesc.HitGroup[HITGROUP_ANY_HIT]);
			Define.HitGroupClosestHit = 
				RtxDesc.HitGroup[HITGROUP_CLOSEST_HIT] == "" ? -1 : AddStringToList(OutStrings, RtxDesc.HitGroup[HITGROUP_CLOSEST_HIT]);
			Define.HitGroupIntersection = 
				RtxDesc.HitGroup[HITGROUP_INTERSECTION] == "" ? -1 : AddStringToList(OutStrings, RtxDesc.HitGroup[HITGROUP_INTERSECTION]);
			Define.MaxAttributeSizeInDepth = RtxDesc.MaxAttributeSizeInBytes;
			Define.MaxPayloadSizeInBytes = RtxDesc.MaxPayloadSizeInBytes;
			Define.MaxTraceRecursionDepth = RtxDesc.MaxTraceRecursionDepth;

			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderRtxPipeline));

			// Write data
			TVector<int32> ExportNameIndices;
			ExportNameIndices.resize(Define.NumExportNames);
			for (int32 i = 0; i < Define.NumExportNames; i++)
			{
				ExportNameIndices[i] = AddStringToList(OutStrings, RtxDesc.ExportNames[i]);
			}
			DataStream.Put(ExportNameIndices.data(), (uint32)(ExportNameIndices.size() * sizeof(int32)));
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
