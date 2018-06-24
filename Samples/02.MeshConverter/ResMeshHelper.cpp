/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include "ResMeshHelper.h"

namespace tix
{
	void TMeshDefine::AddSegment(E_MESH_STREAM_INDEX InStreamType, float* InData, int32 InStrideInByte)
	{
		TI_ASSERT(InStrideInByte % sizeof(float) == 0);
		Segments[InStreamType] = ti_new TResMeshSegment;
		Segments[InStreamType]->Data = InData;
		Segments[InStreamType]->StrideInFloat = InStrideInByte / sizeof(float);
	}

	/////////////////////////////////////////////////////////////////

	TResMeshHelper::TResMeshHelper()
	{
	}

	TResMeshHelper::~TResMeshHelper()
	{
	}

	TMeshDefine& TResMeshHelper::AddMesh(const TString& Name, int32 NumVertices, int32 NumTriangles)
	{
		Meshes.push_back(TMeshDefine(Name, NumVertices, NumTriangles));
		return Meshes.back();
	}

	void TResMeshHelper::OutputMesh(TStream& OutStream, TVector<TString>& OutStrings)
	{
		TResfileChunkHeader ChunkHeader;
		ChunkHeader.ID = TIRES_ID_CHUNK_MESH;
		ChunkHeader.Version = TIRES_VERSION_CHUNK_MESH;
		ChunkHeader.ElementCount = (int32)Meshes.size();

		TStream HeaderStream, DataStream;
		for (int32 m = 0; m < ChunkHeader.ElementCount; ++m)
		{
			const TMeshDefine& Mesh = Meshes[m];
			TI_ASSERT(Mesh.Segments[ESSI_POSITION] != nullptr);

			// init header
			THeaderMesh MeshHeader;
			MeshHeader.StrId_Name = AddStringToList(OutStrings, Mesh.Name);
			MeshHeader.VertexFormat = 0;
			for (int32 s = 0; s < ESSI_TOTAL; ++s)
			{
				if (Mesh.Segments[s] != nullptr)
				{
					MeshHeader.VertexFormat |= (1 << s);
				}
			}
			MeshHeader.VertexCount = Mesh.NumVertices;
			MeshHeader.PrimitiveCount = Mesh.NumTriangles;
			MeshHeader.IndexType = Mesh.NumVertices > 65535 ? EIT_32BIT : EIT_16BIT;
			MeshHeader.Flag = 0;
			vector3df FirstPosition(Mesh.Segments[ESSI_POSITION]->Data[0], Mesh.Segments[ESSI_POSITION]->Data[1], Mesh.Segments[ESSI_POSITION]->Data[2]);
			MeshHeader.BBox.reset(FirstPosition);

			// fill data
			// - vertices
			float* DataPos = Mesh.Segments[ESSI_POSITION] ? Mesh.Segments[ESSI_POSITION]->Data : nullptr;
			float* DataNormal = Mesh.Segments[ESSI_NORMAL] ? Mesh.Segments[ESSI_NORMAL]->Data : nullptr;
			float* DataColor = Mesh.Segments[ESSI_COLOR] ? Mesh.Segments[ESSI_COLOR]->Data : nullptr;
			float* DataUv0 = Mesh.Segments[ESSI_TEXCOORD0] ? Mesh.Segments[ESSI_TEXCOORD0]->Data : nullptr;
			float* DataUv1 = Mesh.Segments[ESSI_TEXCOORD1] ? Mesh.Segments[ESSI_TEXCOORD1]->Data : nullptr;
			float* DataTangent = Mesh.Segments[ESSI_TANGENT] ? Mesh.Segments[ESSI_TANGENT]->Data : nullptr;
			float* DataBI = Mesh.Segments[ESSI_BLENDINDEX] ? Mesh.Segments[ESSI_BLENDINDEX]->Data : nullptr;
			float* DataBW = Mesh.Segments[ESSI_BLENDWEIGHT] ? Mesh.Segments[ESSI_BLENDWEIGHT]->Data : nullptr;
			for (int32 v = 0; v < Mesh.NumVertices; ++v)
			{
				//ESSI_POSITION = 0,
				//ESSI_NORMAL,
				//ESSI_COLOR,
				//ESSI_TEXCOORD0,
				//ESSI_TEXCOORD1,
				//ESSI_TANGENT,
				//ESSI_BLENDINDEX,
				//ESSI_BLENDWEIGHT,
				if (DataPos != nullptr)
				{
					DataStream.Put(DataPos, sizeof(vector3df) * 3);
					DataPos += Mesh.Segments[ESSI_POSITION]->StrideInFloat;
				}
				if (DataNormal != nullptr)
				{
					uint8 NData[4];
					NData[0] = FloatToUNorm(DataNormal[0]);
					NData[1] = FloatToUNorm(DataNormal[1]);
					NData[2] = FloatToUNorm(DataNormal[2]);
					NData[3] = 255;
					DataStream.Put(NData, sizeof(NData));
					DataNormal += Mesh.Segments[ESSI_NORMAL]->StrideInFloat;
				}
				if (DataColor != nullptr)
				{
					uint8 CData[4];
					CData[0] = FloatToUNorm(DataColor[0]);
					CData[1] = FloatToUNorm(DataColor[1]);
					CData[2] = FloatToUNorm(DataColor[2]);
					CData[3] = FloatToUNorm(DataColor[3]);
					DataStream.Put(CData, sizeof(CData));
					DataColor += Mesh.Segments[ESSI_COLOR]->StrideInFloat;
				}
				if (DataUv0 != nullptr)
				{

				}
			}

			TI_TODO("Continue from here. output binary and read it.");
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();
	}
}
