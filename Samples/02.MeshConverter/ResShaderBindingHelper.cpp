/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResShaderBindingHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	//TResShaderBindingHelper::TResShaderBindingHelper()
	//{
	//}

	//TResShaderBindingHelper::~TResShaderBindingHelper()
	//{
	//}

	//void TResShaderBindingHelper::LoadShaderBinding(TJSON& Doc, TStream& OutStream, TVector<TString>& OutStrings)
	//{
	//	TResShaderBindingHelper Helper;

	//	// MPB Name
	//	//TJSONNode MPBName = Doc["name"];
	//	//Helper.SetMaterialInstanceName(MIName.GetString());
	//	
	//	TJSONNode Parameters = Doc["parameters"];
	//	TI_ASSERT(Parameters.IsArray());
	//	Helper.Bindings.resize(Parameters.Size());
	//	for (int32 p = 0; p < Parameters.Size(); ++p)
	//	{
	//		TJSONNode Parameter = Parameters[p];
	//		TJSONNode BindingType = Parameter["type"];
	//		TJSONNode BindingStage = Parameter["stage"];
	//		TJSONNode BindingRegister = Parameter["register"];

	//		int32 ParamSize = Parameter["size"].GetInt();
	//		if (ParamSize == 0)
	//			ParamSize = 1;

	//		TI_ASSERT(ParamSize < 255);
	//		TBindingParamInfo& Binding = Helper.Bindings[p];
	//		Binding.BindingType = (int8)GetBindingType(BindingType.GetString());
	//		Binding.BindingStage = (int8)GetShaderStage(BindingStage.GetString());
	//		Binding.BindingRegister = (uint8)BindingRegister.GetInt();
	//		Binding.BindingSize = (uint8)ParamSize;
	//	}

	//	Helper.OutputShaderBinding(OutStream, OutStrings);
	//}

	//void TResShaderBindingHelper::OutputShaderBinding(TStream& OutStream, TVector<TString>& OutStrings)
	//{
	//	TResfileChunkHeader ChunkHeader;
	//	ChunkHeader.ID = TIRES_ID_CHUNK_SBINDING;
	//	ChunkHeader.Version = TIRES_VERSION_CHUNK_SBINDING;
	//	ChunkHeader.ElementCount = 1;

	//	TStream HeaderStream, DataStream(1024 * 8);
	//	for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
	//	{
	//		THeaderShaderBinding Define;
	//		Define.BindingCount = (int32)Bindings.size();
	//		
	//		// Save Param Bindings
	//		TStream BindingStream;
	//		for (const auto& Binding : Bindings)
	//		{
	//			TBindingParamInfo Info;
	//			Info = Binding;
	//			BindingStream.Put(&Info, sizeof(TBindingParamInfo));
	//		}

	//		// Calculate Binding Crc
	//		Define.BindingCrc = TCrc::MemCrc32(BindingStream.GetBuffer(), BindingStream.GetLength());

	//		// Save header
	//		HeaderStream.Put(&Define, sizeof(THeaderShaderBinding));
	//		// Save Data
	//		DataStream.Put(BindingStream.GetBuffer(), BindingStream.GetLength());
	//	}

	//	ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

	//	OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
	//	FillZero4(OutStream);
	//	OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
	//	OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	//}
}
