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

		Helper.InitBones.resize(Helper.TotalBones);
		for (int32 b = 0; b < Helper.TotalBones; b++)
		{
			TJSONNode JBone = JBones[b];
			ResBoneInfo& Info = Helper.InitBones[b];
			Info.ParentIndex = JBone["parent_index"].GetInt();
			ConvertJArrayToVec3(JBone["translation"], Info.InitPos);
			ConvertJArrayToQuat(JBone["rotation"], Info.InitRot);
			ConvertJArrayToVec3(JBone["scale"], Info.InitScale);
		}

		Helper.CalcInvBindTransform();

		Helper.OutputSkeleton(OutStream, OutStrings);
	}

	matrix4 MakeMatrix(const vector3df& Trans, const quaternion& Rot, const vector3df& Scale)
	{
		matrix4 Result;
		Rot.getMatrix(Result);
		Result.postScale(Scale);
		Result.setTranslation(Trans);

		return Result;
	}

	void TResSkeletonHelper::CalcInvBindTransform()
	{
		TVector<matrix4> BindMatrix;
		BindMatrix.resize(InitBones.size());

		for (int32 b = 0; b < TotalBones; b++)
		{
			const ResBoneInfo& Bone = InitBones[b];
			if (Bone.ParentIndex == -1)
			{
				BindMatrix[b] = MakeMatrix(Bone.InitPos, Bone.InitRot, Bone.InitScale);
			}
			else
			{
				matrix4 ParentMat = BindMatrix[Bone.ParentIndex];
				matrix4 Mat = MakeMatrix(Bone.InitPos, Bone.InitRot, Bone.InitScale);
				BindMatrix[b] = ParentMat * Mat;
			}
		}

		matrix4 testmat;
		testmat.setTranslation(vector3df(1, 2, 3));
		matrix4 invmat;
		testmat.getInverse(invmat);

		ConvertedBones.resize(InitBones.size());
		for (int32 b = 0; b < TotalBones; b++)
		{
			matrix4 InvMat;
			BindMatrix[b].getInverse(InvMat);

			TBoneInitInfo& Bone = ConvertedBones[b];
			Bone.ParentIndex = InitBones[b].ParentIndex;
			Bone.InvPos = InvMat.getTranslation();
			Bone.InvRot = InvMat;
			Bone.InvScale = InvMat.getScale();

			vector3df NewScale = vector3df(1.f / Bone.InvScale.X, 1.f / Bone.InvScale.Y, 1.f / Bone.InvScale.Z);
			InvMat.postScale(NewScale);
			Bone.InvRot = InvMat;
		}
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
			DataStream.Put(ConvertedBones.data(), (uint32)(ConvertedBones.size() * sizeof(TBoneInitInfo)));
		}

		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		FillZero4(OutStream);
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
