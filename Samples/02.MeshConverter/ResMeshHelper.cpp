/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "ResHelper.h"
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

	void TMeshDefine::SetFaces(int32* Indices, int32 Count)
	{
		Faces.Data = Indices;
		Faces.Count = Count;
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

		TStream HeaderStream, DataStream(1024 * 8);
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

			TI_ASSERT(DataPos != nullptr);
			const int32 VertexStart = DataStream.GetLength();
			for (int32 v = 0; v < Mesh.NumVertices; ++v)
			{
				if (DataPos != nullptr)
				{
					vector3df pos(DataPos[0], DataPos[1], DataPos[2]);
					MeshHeader.BBox.addInternalPoint(pos);

					TI_ASSERT(sizeof(vector3df) == TMeshBuffer::SematicSize[ESSI_POSITION]);
					DataStream.Put(DataPos, sizeof(vector3df));
					DataPos += Mesh.Segments[ESSI_POSITION]->StrideInFloat;
				}
				if (DataNormal != nullptr)
				{
					uint8 NData[4];
					NData[0] = FloatToUNorm(DataNormal[0]);
					NData[1] = FloatToUNorm(DataNormal[1]);
					NData[2] = FloatToUNorm(DataNormal[2]);
					NData[3] = 255;

					TI_ASSERT(sizeof(NData) == TMeshBuffer::SematicSize[ESSI_NORMAL]);
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

					TI_ASSERT(sizeof(CData) == TMeshBuffer::SematicSize[ESSI_COLOR]);
					DataStream.Put(CData, sizeof(CData));
					DataColor += Mesh.Segments[ESSI_COLOR]->StrideInFloat;
				}
				if (DataUv0 != nullptr)
				{
					TI_TODO("use full precision uv coord temp, change to half in futher.");
					TI_ASSERT(sizeof(float) * 2 == TMeshBuffer::SematicSize[ESSI_TEXCOORD0]);
					DataStream.Put(DataUv0, sizeof(float) * 2);
					DataUv0 += Mesh.Segments[ESSI_TEXCOORD0]->StrideInFloat;
				}
				if (DataUv1 != nullptr)
				{
					TI_TODO("use full precision uv coord temp, change to half in futher.");
					TI_ASSERT(sizeof(float) * 2 == TMeshBuffer::SematicSize[ESSI_TEXCOORD1]);
					DataStream.Put(DataUv1, sizeof(float) * 2);
					DataUv1 += Mesh.Segments[ESSI_TEXCOORD1]->StrideInFloat;
				}
				if (DataTangent != nullptr)
				{
					uint8 TData[4];
					TData[0] = FloatToUNorm(DataTangent[0]);
					TData[1] = FloatToUNorm(DataTangent[1]);
					TData[2] = FloatToUNorm(DataTangent[2]);
					TData[3] = 255;

					TI_ASSERT(sizeof(TData) == TMeshBuffer::SematicSize[ESSI_TANGENT]);
					DataStream.Put(TData, sizeof(TData));
					DataTangent += Mesh.Segments[ESSI_TANGENT]->StrideInFloat;
				}
				if (DataBI != nullptr)
				{
					TI_TODO("use float precision blend index temp, change to uint8 in futher.");
					TI_ASSERT(sizeof(float) * 4 == TMeshBuffer::SematicSize[ESSI_BLENDINDEX]);
					DataStream.Put(DataBI, sizeof(float) * 4);
					DataBI += Mesh.Segments[ESSI_BLENDINDEX]->StrideInFloat;
				}
				if (DataBW != nullptr)
				{
					TI_ASSERT(sizeof(float) * 4 == TMeshBuffer::SematicSize[ESSI_BLENDWEIGHT]);
					DataStream.Put(DataBW, sizeof(float) * 4);
					DataBW += Mesh.Segments[ESSI_BLENDWEIGHT]->StrideInFloat;
				}
			}
			const int32 VertexEnd = DataStream.GetLength();

			// 8 bytes align
			TI_ASSERT((VertexEnd - VertexStart) % 4 == 0);
			FillZero8(DataStream);

			// - Indices
			const int32 IndexStart = DataStream.GetLength();
			TI_ASSERT(Mesh.Faces.Count == MeshHeader.PrimitiveCount * 3);
			if (MeshHeader.IndexType == EIT_16BIT)
			{
				for (int32 i = 0; i < Mesh.Faces.Count; ++i)
				{
					uint16 Index = (uint16)Mesh.Faces.Data[i];
					DataStream.Put(&Index, sizeof(uint16));
				}
			}
			else
			{
				for (int32 i = 0; i < Mesh.Faces.Count; ++i)
				{
					uint32 Index = (uint32)Mesh.Faces.Data[i];
					DataStream.Put(&Index, sizeof(uint32));
				}
			}
			const int32 IndexEnd = DataStream.GetLength();
			FillZero8(DataStream);

			// Fill header
			const int32 HeaderStart = HeaderStream.GetLength();
			HeaderStream.Put(&MeshHeader, sizeof(THeaderMesh));
			const int32 HeaderEnd = HeaderStream.GetLength();
			FillZero8(HeaderStream);
		}
		ChunkHeader.ChunkSize = HeaderStream.GetLength() + DataStream.GetLength();

		OutStream.Put(&ChunkHeader, sizeof(TResfileChunkHeader));
		OutStream.Put(HeaderStream.GetBuffer(), HeaderStream.GetLength());
		OutStream.Put(DataStream.GetBuffer(), DataStream.GetLength());
	}
}
