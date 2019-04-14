/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResInstancesHelper.h"

namespace tix
{
	TResInstancesHelper::TResInstancesHelper()
	{
	}

	TResInstancesHelper::~TResInstancesHelper()
	{
	}

	bool TResInstancesHelper::LoadInstances(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResInstancesHelper Helper;

		// Instances Name
		TString InstancesName = Doc["name"].GetString();

		// linked material
		TJSONNode JLinkedMesh = Doc["linked_mesh"];
		Helper.LinkedMesh = JLinkedMesh.GetString();

		TJSONNode JInstances = Doc["instances"];
		TI_ASSERT(JInstances.IsArray());
		Helper.Instances.reserve(JInstances.Size());
		for (int32 i = 0; i < JInstances.Size(); ++i)
		{
			TJSONNode JInstance = JInstances[i];
			TJSONNode JPosition = JInstance["position"];
			TJSONNode JRotation = JInstance["rotation"];
			TJSONNode JScale = JInstance["scale"];

			TResInstance Ins;
			Ins.Position[0] = JPosition[0].GetFloat();
			Ins.Position[1] = JPosition[1].GetFloat();
			Ins.Position[2] = JPosition[2].GetFloat();
			Ins.Rotation[0] = JRotation[0].GetFloat();
			Ins.Rotation[1] = JRotation[1].GetFloat();
			Ins.Rotation[2] = JRotation[2].GetFloat();
			Ins.Rotation[3] = JRotation[3].GetFloat();
			Ins.Scale[0] = JScale[0].GetFloat();
			Ins.Scale[1] = JScale[1].GetFloat();
			Ins.Scale[2] = JScale[2].GetFloat();
			Helper.Instances.push_back(Ins);
		}

		Helper.OutputInstances(OutStream, OutStrings);
		return true;
	}
	
	void TResInstancesHelper::OutputInstances(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_INSTANCES;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_INSTANCES;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			// init header
			THeaderInstances InstancesHeader;
			InstancesHeader.LinkedMeshNameIndex = AddStringToList(OutStrings, LinkedMesh);
			InstancesHeader.NumInstances = (int32)Instances.size();

			HeaderStream.Put(&InstancesHeader, sizeof(THeaderInstances));

			for (int32 i = 0; i < InstancesHeader.NumInstances; ++i)
			{
				const TResInstance& Ins = Instances[i];
				DataStream.Put(&Ins, sizeof(TResInstance));
			}
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
