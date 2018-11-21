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
		TI_ASSERT(0);
		//TResfileChunkHeader ChunkHeader;
		//ChunkHeader.ID = TIRES_ID_CHUNK_MINSTANCE;
		//ChunkHeader.Version = TIRES_VERSION_CHUNK_MINSTANCE;
		//ChunkHeader.ElementCount = 1;

		//TStream HeaderStream, DataStream(1024 * 8);
		//for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		//{
		//	THeaderMaterialInstance Define;
		//	Define.NameIndex = AddStringToList(OutStrings, InstanceName);
		//	Define.LinkedMaterialIndex = AddStringToList(OutStrings, LinkedMaterial);
		//	Define.ParamCount = (int32)(ValueParameters.size() + TextureParameters.size());
		//	
		//	// Save header
		//	HeaderStream.Put(&Define, sizeof(THeaderMaterialInstance));

		//	// Save Parameters formats
		//	TStream SName, SType, SValue;
		//	// Value params first
		//	for (const auto& Param : ValueParameters)
		//	{
		//		int32 ParamNameIndex = AddStringToList(OutStrings, Param.ParamName);
		//		SName.Put(&ParamNameIndex, sizeof(int32));
		//		SType.Put(&Param.ParamType, sizeof(uint8));
		//		switch (Param.ParamType)
		//		{
		//		case MIPT_INT:
		//		case MIPT_FLOAT:
		//			SValue.Put(&Param.ParamValue.ValueFloat, sizeof(float));
		//			break;
		//		case MIPT_INT4:
		//		case MIPT_FLOAT4:
		//			SValue.Put(&Param.ParamValue.ValueQuat, sizeof(float) * 4);
		//			break;
		//		default:
		//			printf("Invalid param type %d for %s.\n", Param.ParamType, InstanceName.c_str());
		//			break;
		//		}
		//	}
		//	// Then texture params
		//	for (const auto& Param : TextureParameters)
		//	{
		//		int32 ParamNameIndex = AddStringToList(OutStrings, Param.ParamName);
		//		SName.Put(&ParamNameIndex, sizeof(int32));
		//		SType.Put(&Param.ParamType, sizeof(uint8));
		//		switch (Param.ParamType)
		//		{
		//		case MIPT_TEXTURE:
		//		{
		//			int32 TextureNameIndex = AddStringToList(OutStrings, Param.ParamValue.ValueString);
		//			SValue.Put(&TextureNameIndex, sizeof(int32));
		//		}
		//		break;
		//		default:
		//			printf("Invalid param type %d for %s.\n", Param.ParamType, InstanceName.c_str());
		//			break;
		//		}
		//	}
		//	FillZero4(SType);
		//	DataStream.Put(SName.GetBuffer(), SName.GetLength());
		//	DataStream.Put(SType.GetBuffer(), SType.GetLength());
		//	DataStream.Put(SValue.GetBuffer(), SValue.GetLength());
		//}

		//ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		//OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		//FillZero4(OutStream);
		//OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		//OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
