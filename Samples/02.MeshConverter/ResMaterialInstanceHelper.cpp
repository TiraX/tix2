/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResMaterialInstanceHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

namespace tix
{
	TResMaterialInstanceHelper::TResMaterialInstanceHelper()
	{
	}

	TResMaterialInstanceHelper::~TResMaterialInstanceHelper()
	{
	}

	void TResMaterialInstanceHelper::LoadMaterialInstance(const TString& Filename, TStream& OutStream, TVector<TString>& OutStrings)
	{
		TFile f;
		if (!f.Open(Filename, EFA_READ))
			return;

		int8* content = ti_new int8[f.GetSize() + 1];
		f.Read(content, f.GetSize(), f.GetSize());
		content[f.GetSize()] = 0;
		f.Close();

		Document tjs;
		tjs.Parse(content);
		ti_delete[] content;

		TResMaterialInstanceHelper Helper;

		// MI Name
		Value& MIName = tjs["name"];
		Helper.SetMaterialInstanceName(MIName.GetString());

		// linked material
		Value& LinkedMaterial = tjs["linked_material"];
		Helper.SetMaterialRes(LinkedMaterial.GetString());

		Value& Parameters = tjs["parameters"];
		TI_ASSERT(Parameters.IsObject()); 
		for (Value::ConstMemberIterator itr = Parameters.MemberBegin();
			itr != Parameters.MemberEnd(); ++itr)
		{
			TString ParamName = itr->name.GetString();
			Value& Param = Parameters[ParamName.c_str()];
			TString ParamType = Param["type"].GetString();
			Value& ParamValue = Param["value"];
			if (ParamType == "int")
			{
				Helper.AddParameter(ParamName, ParamValue.GetInt());
			}
			else if (ParamType == "float")
			{
				Helper.AddParameter(ParamName, ParamValue.GetFloat());
			}
			else if (ParamType == "float4")
			{
				TI_ASSERT(ParamValue.IsArray() && ParamValue.Size() <= 4);
				quaternion q;
				for (SizeType pv = 0; pv < ParamValue.Size(); ++pv)
					q[pv] = ParamValue[pv].GetFloat();
				Helper.AddParameter(ParamName, q);
			}
			else if (ParamType == "texture")
			{
				TI_ASSERT(ParamValue.IsString());
				Helper.AddParameter(ParamName, ParamValue.GetString());
			}
			else
			{
				TI_ASSERT(0);
			}
		}

		Helper.OutputMaterialInstance(OutStream, OutStrings);
	}

	void TResMaterialInstanceHelper::SetMaterialInstanceName(const TString& InInstanceName)
	{
		InstanceName = InInstanceName;
	}

	void TResMaterialInstanceHelper::SetMaterialRes(const TString& MaterialName)
	{
		LinkedMaterial = MaterialName;
	}

	bool TResMaterialInstanceHelper::IsParamExisted(const TString& InParamName)
	{
		for (const auto& Param : Parameters)
		{
			if (Param.ParamName == InParamName)
			{
				printf("Duplicated param [%s] in %s.\n", InParamName.c_str(), InstanceName.c_str());
				return true;
			}
		}
		return false;
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& InParamName, int32 Value)
	{
		if (IsParamExisted(InParamName))
			return;
		TMIParam V(InParamName);
		V.ParamValue.ValueInt = Value;
		V.ParamType = MIPT_INT;
		Parameters.push_back(V);
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& InParamName, float Value)
	{
		if (IsParamExisted(InParamName))
			return;
		TMIParam V(InParamName);
		V.ParamValue.ValueFloat = Value;
		V.ParamType = MIPT_FLOAT;
		Parameters.push_back(V);
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& InParamName, const vector3df& Value)
	{
		if (IsParamExisted(InParamName))
			return;
		TMIParam V(InParamName);
		V.ParamValue.ValueVec = Value;
		V.ParamType = MIPT_FLOAT4;
		Parameters.push_back(V);
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& InParamName, const quaternion& Value)
	{
		if (IsParamExisted(InParamName))
			return;
		TMIParam V(InParamName);
		V.ParamValue.ValueQuat = Value;
		V.ParamType = MIPT_FLOAT4;
		Parameters.push_back(V);
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& InParamName, const SColorf& Value)
	{
		if (IsParamExisted(InParamName))
			return;
		TMIParam V(InParamName);
		V.ParamValue.ValueClr = Value;
		V.ParamType = MIPT_FLOAT4;
		Parameters.push_back(V);
	}

	void TResMaterialInstanceHelper::AddParameter(const TString& InParamName, const TString& Value)
	{
		if (IsParamExisted(InParamName))
			return;
		TMIParam V(InParamName);
		V.ParamValue.ValueString = Value;
		V.ParamType = MIPT_TEXTURE;
		Parameters.push_back(V);
	}
	
	void TResMaterialInstanceHelper::OutputMaterialInstance(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_MINSTANCE;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_MINSTANCE;
		ChunkHeader.ElementCount = 1;

		TStream HeaderStream, DataStream(1024 * 8);
		for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		{
			THeaderMaterialInstance Define;
			Define.NameIndex = AddStringToList(OutStrings, InstanceName);
			Define.LinkedMaterialIndex = AddStringToList(OutStrings, LinkedMaterial);
			Define.ParamCount = (int32)Parameters.size();
			
			// Save header
			HeaderStream.Put(&Define, sizeof(THeaderMaterialInstance));

			// Save Parameters formats
			TStream SName, SType, SValue;
			for (const auto& Param : Parameters)
			{
				int32 ParamNameIndex = AddStringToList(OutStrings, Param.ParamName);
				SName.Put(&ParamNameIndex, sizeof(int32));
				SType.Put(&Param.ParamType, sizeof(uint8));
				switch (Param.ParamType)
				{
				case MIPT_INT:
				case MIPT_FLOAT:
					SValue.Put(&Param.ParamValue.ValueFloat, sizeof(float));
					break;
				case MIPT_INT4:
				case MIPT_FLOAT4:
					SValue.Put(&Param.ParamValue.ValueQuat, sizeof(float) * 4);
					break;
				case MIPT_TEXTURE:
				{
					int32 TextureNameIndex = AddStringToList(OutStrings, Param.ParamValue.ValueString);
					SValue.Put(&TextureNameIndex, sizeof(int32));
				}
					break;
				default:
					printf("Invalid param type %d for %s.\n", Param.ParamType, InstanceName.c_str());
					break;
				}
			}
			FillZero4(SType);
			DataStream.Put(SName.GetBuffer(), SName.GetLength());
			DataStream.Put(SType.GetBuffer(), SType.GetLength());
			DataStream.Put(SValue.GetBuffer(), SValue.GetLength());
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
