/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMaterialBindingHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	TResMaterialBindingHelper::TResMaterialBindingHelper()
	{
	}

	TResMaterialBindingHelper::~TResMaterialBindingHelper()
	{
	}

	void TResMaterialBindingHelper::LoadMaterialParameterBinding(rapidjson::Document& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResMaterialBindingHelper Helper;

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

			TMaterialParamBinding& Binding = Helper.Bindings[p];
			Binding.BindingType = GetBindingType(BindingType.GetString());
			Binding.BindingStage = GetShaderStage(BindingStage.GetString());
			Binding.Size = ParamSize;
		}

		Helper.OutputMaterialParameterBinding(OutStream, OutStrings);
	}

	void TResMaterialBindingHelper::OutputMaterialParameterBinding(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_MBINDING;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_MBINDING;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderMaterialBinding Define;
			Define.BindingCount = (int32)Bindings.size();

			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderMaterialBinding));

			// Save Param Bindings
			for (const auto& Binding : Bindings)
			{
				TBindingInfo Info;
				Info.BindingType = (int8)Binding.BindingType;
				Info.BindingStage = (int8)Binding.BindingStage;
				Info.BindingSize = (int8)Binding.Size;
				DataStream.Put(&Info, sizeof(TBindingInfo));
			}
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
