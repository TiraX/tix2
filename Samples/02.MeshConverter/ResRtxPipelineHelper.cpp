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
		Helper.ExportNames.resize(JExportNames.Size());
		for (int32 i = 0; i < JExportNames.Size(); ++i)
		{
			Helper.ExportNames[i] = JExportNames[i].GetString();
		}

		// hit group
		TJSONNode JHitGroup = Doc["hit_group"];
		TI_ASSERT(JHitGroup.IsArray() && JHitGroup.Size() == HITGROUP_NUM);
		Helper.HitGroupShader[HITGROUP_ANY_HIT] = JHitGroup[HITGROUP_ANY_HIT].GetString();
		Helper.HitGroupShader[HITGROUP_CLOSEST_HIT] = JHitGroup[HITGROUP_CLOSEST_HIT].GetString();
		Helper.HitGroupShader[HITGROUP_INTERSECTION] = JHitGroup[HITGROUP_INTERSECTION].GetString();

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
			Define.NumExportNames = (int32)ExportNames.size();
			Define.HitGroupAnyHit = AddStringToList(OutStrings, HitGroupShader[HITGROUP_ANY_HIT]);
			Define.HitGroupClosestHit = AddStringToList(OutStrings, HitGroupShader[HITGROUP_CLOSEST_HIT]);
			Define.HitGroupIntersection = AddStringToList(OutStrings, HitGroupShader[HITGROUP_INTERSECTION]);

			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderRtxPipeline));

			// Write data
			TVector<int32> ExportNameIndices;
			ExportNameIndices.resize(Define.NumExportNames);
			for (int32 i = 0; i < Define.NumExportNames; i++)
			{
				if (ExportNames[i] != "")
					ExportNameIndices[i] = AddStringToList(OutStrings, ExportNames[i]);
				else
					ExportNameIndices[i] = -1;
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
