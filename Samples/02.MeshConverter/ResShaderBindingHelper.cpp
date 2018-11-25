/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResShaderBindingHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	TResShaderBindingHelper::TResShaderBindingHelper()
	{
	}

	TResShaderBindingHelper::~TResShaderBindingHelper()
	{
	}

	void TResShaderBindingHelper::LoadShaderBinding(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResShaderBindingHelper Helper;

		// MPB Name
		Value& MPBName = Doc["name"];
		//Helper.SetMaterialInstanceName(MIName.GetString());
		
		Value& Parameters = Doc["parameters"];
		TI_ASSERT(Parameters.IsArray());
		Helper.Bindings.resize(Parameters.Size());
		for (SizeType p = 0; p < Parameters.Size(); ++p)
		{
			Value& Parameter = Parameters[p];
			Value& BindingType = Parameter["type"];
			Value& BindingStage = Parameter["stage"];

			int32 ParamSize = 1;
			if (Parameter.FindMember("size") != Parameter.MemberEnd())
			{
				ParamSize = Parameter["size"].GetInt();
			}

			TI_ASSERT(ParamSize < 255);
			TBindingParamInfo& Binding = Helper.Bindings[p];
			Binding.BindingType = (int8)GetBindingType(BindingType.GetString());
			Binding.BindingStage = (int8)GetShaderStage(BindingStage.GetString());
			Binding.BindingSize = (uint8)ParamSize;
		}

		Helper.OutputShaderBinding(OutStream, OutStrings);
	}

	void TResShaderBindingHelper::OutputShaderBinding(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_SBINDING;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_SBINDING;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderShaderBinding Define;
			Define.BindingCount = (int32)Bindings.size();
			
			// Save Param Bindings
			TStream BindingStream;
			for (const auto& Binding : Bindings)
			{
				TBindingParamInfo Info;
				Info = Binding;
				BindingStream.Put(&Info, sizeof(TBindingParamInfo));
			}

			// Calculate Binding Crc
			Define.BindingCrc = TCrc::MemCrc32(BindingStream.GetBuffer(), BindingStream.GetLength());

			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderShaderBinding));
			// Save Data
			DataStream.Put(BindingStream.GetBuffer(), BindingStream.GetLength());
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
