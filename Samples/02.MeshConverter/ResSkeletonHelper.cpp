/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
#include "ResSkeletonHelper.h"
#include "rapidjson/document.h"

using namespace rapidjson;

void ConvertJArrayToVec3(const TJSONNode& JArray, vector3df& V3)
{
	TI_ASSERT(JArray.IsArray() && JArray.Size() == 3);
	int32 JArraySize = JArray.Size();
	V3.X = JArray[0].GetFloat();
	V3.Y = JArray[1].GetFloat();
	V3.Z = JArray[2].GetFloat();
}
void ConvertJArrayToQuat(const TJSONNode& JArray, quaternion& Q4)
{
	TI_ASSERT(JArray.IsArray() && JArray.Size() == 4);
	int32 JArraySize = JArray.Size();
	Q4.X = JArray[0].GetFloat();
	Q4.Y = JArray[1].GetFloat();
	Q4.Z = JArray[2].GetFloat();
	Q4.W = JArray[3].GetFloat();
}

namespace tix
{
	TResSkeletonHelper::TResSkeletonHelper()
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
			FBoneInfo& Info = Helper.Bones[b];
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
		//for (int32 t = 0; t < ChunkHeader.ElementCount; ++t)
		//{
		//	THeaderMaterial Define;
		//	for (int32 s = 0; s < ESS_COUNT; ++s)
		//	{
		//		Define.ShaderNames[s] = AddStringToList(OutStrings, ShaderNames[s]);
		//		Define.ShaderCodeLength[s] = ShaderCodes[s].GetLength();
		//	}

		//	Define.Flags = PipelineDesc.Flags;
		//	Define.BlendMode = BlendMode;
		//	Define.BlendState = PipelineDesc.BlendState;
		//	Define.RasterizerDesc = PipelineDesc.RasterizerDesc;
		//	Define.DepthStencilDesc = PipelineDesc.DepthStencilDesc;
		//	Define.VsFormat = PipelineDesc.VsFormat;
		//	Define.InsFormat = PipelineDesc.InsFormat;
		//	Define.PrmitiveType = PipelineDesc.PrimitiveType;

		//	int32 cb = 0;
		//	for (; cb < ERTC_COUNT; ++cb)
		//	{
		//		Define.ColorBuffers[cb] = PipelineDesc.RTFormats[cb];
		//	}
		//	Define.DepthBuffer = PipelineDesc.DepthFormat;

		//	// Save header
		//	HeaderStream.Put(&Define, sizeof(THeaderMaterial));

		//	// Write codes
		//	for (int32 s = 0; s < ESS_COUNT; ++s)
		//	{
		//		if (ShaderCodes[s].GetLength() > 0)
		//		{
		//			DataStream.Put(ShaderCodes[s].GetBuffer(), ShaderCodes[s].GetLength());
		//			FillZero4(DataStream);
		//		}
		//	}
		//}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
